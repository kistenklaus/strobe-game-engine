#include <strobe/flow/detail/op_stream.hpp>

namespace strobe::flow::detail {

void OpStream::validate() const {
  if (m_defined.size() != m_resources.size()) {
    throw std::logic_error("resource state table is inconsistent");
  }

  for (std::size_t index = 0; index < m_resources.size(); ++index) {
    validate_resource(index, m_resources[index]);
  }
  for (std::size_t index = 0; index < m_phis.size(); ++index) {
    validate_phi(index, m_phis[index]);
  }
  for (std::size_t index = 0; index < m_cutoffs.size(); ++index) {
    validate_cutoff(index, m_cutoffs[index]);
  }
  for (std::size_t index = 0; index < m_externals.size(); ++index) {
    validate_external(index, m_externals[index]);
  }
  for (const auto &branch : m_ifs) {
    if (branch.condition >= m_resources.size() ||
        m_resources[branch.condition].type != type_id<bool>() ||
        branch.begin > branch.alternative || branch.alternative > branch.end ||
        branch.end > m_ops.size()) {
      throw std::logic_error("if metadata is inconsistent");
    }
  }
  for (std::size_t index = 0; index < m_ops.size(); ++index) {
    validate_operation(index, m_ops[index]);
  }
}

std::size_t OpStream::yield() {
  Operation operation;
  operation.kind = OperationKind::yield;
  operation.invoke = [](Lifetime, span<void *const> storage) {
    validate_storage(storage, 0);
  };
  return append(std::move(operation));
}

std::size_t OpStream::acquire(std::string_view name) {
  if (name.empty()) {
    throw std::invalid_argument("acquire name cannot be empty");
  }
  Operation operation;
  operation.kind = OperationKind::acquire;
  operation.invoke = [](Lifetime, span<void *const> storage) {
    validate_storage(storage, 0);
  };
  const auto operation_index = append(std::move(operation));
  m_acquires.push_back(AcquireInfo{
      .operation = operation_index,
      .name = String<allocator_ref>(name, allocator_ref{}),
  });
  return operation_index;
}

std::size_t OpStream::size() const noexcept { return m_ops.size(); }

span<const std::size_t> OpStream::predecessors(std::size_t index) const {
  if (index >= m_ops.size()) {
    throw std::out_of_range("operation index is out of range");
  }
  const auto &dependencies = m_ops[index].predecessors;
  return {dependencies.data(), dependencies.size()};
}

const Vector<Operation, allocator_ref> &OpStream::ops() const noexcept {
  return m_ops;
}

const Vector<ResourceInfo, allocator_ref> &
OpStream::resources() const noexcept {
  return m_resources;
}

const Vector<PhiInfo, allocator_ref> &OpStream::phis() const noexcept {
  return m_phis;
}

const Vector<CutoffInfo, allocator_ref> &OpStream::cutoffs() const noexcept {
  return m_cutoffs;
}

const Vector<AcquireInfo, allocator_ref> &OpStream::acquires() const noexcept {
  return m_acquires;
}

const Vector<IfInfo, allocator_ref> &OpStream::ifs() const noexcept {
  return m_ifs;
}

std::move_only_function<bool(const void *, const void *) const>
OpStream::release_cutoff_equal(std::size_t cutoff) {
  if (cutoff >= m_cutoffs.size()) {
    throw std::out_of_range("cutoff index is out of range");
  }
  return std::move(m_cutoffs[cutoff].equal);
}

std::move_only_function<void(void *, const void *) const>
OpStream::release_cutoff_copy(std::size_t cutoff) {
  if (cutoff >= m_cutoffs.size()) {
    throw std::out_of_range("cutoff index is out of range");
  }
  return std::move(m_cutoffs[cutoff].copy_construct);
}

const Vector<ExternalInfo, allocator_ref> &
OpStream::externals() const noexcept {
  return m_externals;
}

std::move_only_function<void(Lifetime, span<void *const>)>
OpStream::release_callback(std::size_t operation) {
  if (operation >= m_ops.size()) {
    throw std::out_of_range("operation callback index is out of range");
  }
  return std::move(m_ops[operation].invoke);
}

void OpStream::validate_resource(std::size_t index,
                                 const ResourceInfo &resource) const {
  const auto definitions =
      static_cast<unsigned>(resource.construction.has_value()) +
      static_cast<unsigned>(resource.phi.has_value()) +
      static_cast<unsigned>(resource.cutoff.has_value()) +
      static_cast<unsigned>(resource.external.has_value());
  if (definitions == 0) {
    throw std::logic_error("resource is missing its definition");
  }
  if (definitions != 1) {
    throw std::logic_error("resource has multiple definitions");
  }
  if (resource.construction) {
    if (*resource.construction >= m_ops.size()) {
      throw std::logic_error("resource construction index is out of range");
    }
    const auto &operation = m_ops[*resource.construction];
    if (operation.kind != OperationKind::construct ||
        operation.accesses.empty() || operation.accesses[0].resource != index ||
        operation.accesses[0].mode != AccessMode::write) {
      throw std::logic_error("resource construction link is inconsistent");
    }
    if (resource.destroy == nullptr || resource.size == 0 ||
        resource.alignment == 0) {
      throw std::logic_error("owned resource metadata is incomplete");
    }
  }
  if (resource.phi && (*resource.phi >= m_phis.size() ||
                       m_phis[*resource.phi].output != index)) {
    throw std::logic_error("resource phi link is inconsistent");
  }
  if (resource.cutoff && (*resource.cutoff >= m_cutoffs.size() ||
                          m_cutoffs[*resource.cutoff].output != index)) {
    throw std::logic_error("resource cutoff link is inconsistent");
  }
  if (resource.external &&
      (*resource.external >= m_externals.size() ||
       m_externals[*resource.external].resource != index)) {
    throw std::logic_error("resource external link is inconsistent");
  }
}

void OpStream::validate_phi(std::size_t index, const PhiInfo &phi) const {
  if (phi.output >= m_resources.size() || phi.initial >= m_resources.size()) {
    throw std::logic_error("phi resource index is out of range");
  }
  if (phi.position > m_ops.size()) {
    throw std::logic_error("phi position is out of range");
  }
  if (index != 0 && m_phis[index - 1].position > phi.position) {
    throw std::logic_error("phi positions are not ordered");
  }
  if (!phi.feedback) {
    throw std::logic_error("phi is missing its feedback resource");
  }
  if (!phi.feedback_position || *phi.feedback_position > m_ops.size()) {
    throw std::logic_error("phi feedback position is invalid");
  }
  if (*phi.feedback >= m_resources.size()) {
    throw std::logic_error("phi feedback resource is out of range");
  }
  if (phi.condition && (*phi.condition >= m_resources.size() ||
                        m_resources[*phi.condition].type != type_id<bool>())) {
    throw std::logic_error("phi feedback condition is invalid");
  }
  if (m_resources[phi.output].phi != index) {
    throw std::logic_error("phi output link is inconsistent");
  }
}

void OpStream::validate_cutoff(std::size_t index,
                               const CutoffInfo &cutoff) const {
  if (cutoff.input >= m_resources.size() ||
      cutoff.output >= m_resources.size() || cutoff.operation >= m_ops.size()) {
    throw std::logic_error("cutoff index is out of range");
  }
  if (m_resources[cutoff.output].cutoff != index || !cutoff.equal ||
      !cutoff.copy_construct || cutoff.destroy == nullptr || cutoff.size == 0 ||
      cutoff.alignment == 0) {
    throw std::logic_error("cutoff metadata is inconsistent");
  }
  const auto &operation = m_ops[cutoff.operation];
  if (operation.kind != OperationKind::cutoff ||
      operation.accesses.size() != 2 ||
      operation.accesses[0].resource != cutoff.input ||
      operation.accesses[0].mode != AccessMode::read ||
      operation.accesses[1].resource != cutoff.output ||
      operation.accesses[1].mode != AccessMode::write) {
    throw std::logic_error("cutoff operation is inconsistent");
  }
}

void OpStream::validate_external(std::size_t index,
                                 const ExternalInfo &external) const {
  if (external.resource >= m_resources.size() || external.name.empty() ||
      m_resources[external.resource].external != index) {
    throw std::logic_error("external resource metadata is inconsistent");
  }
  for (std::size_t earlier = 0; earlier < index; ++earlier) {
    if (m_externals[earlier].name == external.name) {
      throw std::logic_error("external resource names are not unique");
    }
  }
}

void OpStream::validate_operation(std::size_t index,
                                  const Operation &operation) const {
  if (!operation.invoke && !operation.external) {
    throw std::logic_error("operation has no invocation callback");
  }
  if (operation.invoke && operation.external) {
    throw std::logic_error("operation has multiple invocation sources");
  }
  if ((operation.kind == OperationKind::callback ||
       operation.kind == OperationKind::construct) &&
      (operation.argument_types.size() != operation.accesses.size() ||
       operation.argument_accesses.size() != operation.accesses.size())) {
    throw std::logic_error("operation argument metadata is inconsistent");
  }
  validate_unique_accesses(
      {operation.accesses.data(), operation.accesses.size()});
  for (const auto &access : operation.accesses) {
    if (access.resource >= m_resources.size()) {
      throw std::logic_error("operation resource access is out of range");
    }
  }
  std::size_t previous = 0;
  bool first = true;
  for (const auto predecessor : operation.predecessors) {
    if (predecessor >= index || (!first && predecessor <= previous)) {
      throw std::logic_error("operation predecessor list is invalid");
    }
    previous = predecessor;
    first = false;
  }

  switch (operation.kind) {
  case OperationKind::callback:
    break;
  case OperationKind::construct:
    if (operation.accesses.empty() ||
        operation.accesses[0].mode != AccessMode::write) {
      throw std::logic_error("construction operation has no output");
    }
    break;
  case OperationKind::cutoff:
    if (operation.accesses.size() != 2) {
      throw std::logic_error("cutoff operation has invalid accesses");
    }
    break;
  case OperationKind::discard:
    if (operation.accesses.size() != 1 ||
        operation.accesses[0].mode != AccessMode::write) {
      throw std::logic_error("discard operation has invalid accesses");
    }
    break;
  case OperationKind::acquire:
    if (!operation.accesses.empty()) {
      throw std::logic_error("acquire operation cannot access resources");
    }
    if (std::none_of(m_acquires.begin(), m_acquires.end(),
                     [&](const AcquireInfo &acquire) {
                       return acquire.operation == index &&
                              !acquire.name.empty();
                     })) {
      throw std::logic_error("acquire operation has no named semaphore");
    }
    break;
  case OperationKind::yield:
    if (!operation.accesses.empty()) {
      throw std::logic_error("yield operation cannot access resources");
    }
    break;
  case OperationKind::phi:
    throw std::logic_error("phi cannot be a recorded operation");
  }
}

void OpStream::validate_storage(span<void *const> storage, std::size_t count) {
  if (storage.size() != count ||
      std::any_of(storage.begin(), storage.end(),
                  [](void *pointer) { return pointer == nullptr; })) {
    throw std::invalid_argument("operation requires storage for every access");
  }
}

void OpStream::validate_unique_accesses(span<const AccessInfo> accesses) {
  for (std::size_t left = 0; left < accesses.size(); ++left) {
    for (std::size_t right = left + 1; right < accesses.size(); ++right) {
      if (accesses[left].resource == accesses[right].resource) {
        throw std::logic_error(
            "an operation cannot access the same resource more than once");
      }
    }
  }
}

void OpStream::validate_external_op(const ExternalOp &source) {
  if (source.library_id.empty() || source.symbol.empty() ||
      source.abi_version == 0) {
    throw std::invalid_argument(
        "external op requires a library id, symbol, and ABI version");
  }
}

std::size_t OpStream::append(Operation operation) {
  for (std::size_t index = 0; index < m_ops.size(); ++index) {
    const auto conflicts = [&](const AccessInfo &earlier) {
      return std::any_of(operation.accesses.begin(), operation.accesses.end(),
                         [&](const AccessInfo &later) {
                           return earlier.resource == later.resource &&
                                  (earlier.mode != AccessMode::read ||
                                   later.mode != AccessMode::read);
                         });
    };
    if (std::any_of(m_ops[index].accesses.begin(), m_ops[index].accesses.end(),
                    conflicts)) {
      operation.predecessors.push_back(index);
    }
  }
  const auto index = m_ops.size();
  m_ops.push_back(std::move(operation));
  return index;
}

} // namespace strobe::flow::detail
