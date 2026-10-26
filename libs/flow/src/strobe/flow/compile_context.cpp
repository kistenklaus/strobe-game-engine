#include <strobe/flow/compile_context.hpp>

namespace strobe::flow {

namespace {

bool valid_resolved_operation(
    const detail::ResolvedOperation &operation) noexcept {
  const bool has_runtime = static_cast<bool>(operation.runtime);
  const bool has_external = operation.external.has_value();
  if (has_runtime == has_external ||
      operation.argument_types.size() != operation.argument_accesses.size()) {
    return false;
  }
  std::size_t grouped = 0;
  std::size_t elements = 0;
  for (const auto group : operation.argument_groups) {
    if (group.offset != grouped ||
        grouped > operation.argument_types.size() ||
        group.count > operation.argument_types.size() - grouped ||
        group.element_offset != elements ||
        elements > operation.argument_elements.size() ||
        group.element_count > operation.argument_elements.size() - elements) {
      return false;
    }
    std::uint32_t present = 0;
    for (std::uint32_t index = 0; index < group.element_count; ++index) {
      const auto element = operation.argument_elements[elements + index];
      if (element != null_argument_element &&
          element != group.offset + present++) {
        return false;
      }
    }
    if (present != group.count) {
      return false;
    }
    grouped += group.count;
    elements += group.element_count;
  }
  if (grouped != operation.argument_types.size() ||
      elements != operation.argument_elements.size()) {
    return false;
  }
  for (const auto &immediate : operation.immediates) {
    if (immediate.type.value == 0 || immediate.element_size == 0 ||
        immediate.element_count >
            std::numeric_limits<std::size_t>::max() / immediate.element_size ||
        immediate.size != immediate.element_size * immediate.element_count ||
        immediate.alignment == 0 ||
        (immediate.alignment & (immediate.alignment - 1)) != 0 ||
        immediate.offset > operation.immediate_data.size() ||
        immediate.size > operation.immediate_data.size() - immediate.offset ||
        immediate.offset % immediate.alignment != 0) {
      return false;
    }
  }
  return !has_external || (!operation.external->library_id.empty() &&
                           !operation.external->symbol.empty() &&
                           operation.external->abi_version != 0);
}

} // namespace

CompileContext::CompileContext(CompileContext &&other) noexcept
    : m_entries(std::move(other.m_entries)),
      m_resolved_operations(std::move(other.m_resolved_operations)) {}

CompileContext &CompileContext::operator=(CompileContext &&other) noexcept {
  if (this != &other) {
    clear();
    m_entries = std::move(other.m_entries);
    m_resolved_operations = std::move(other.m_resolved_operations);
  }
  return *this;
}

CompileContext::~CompileContext() { clear(); }

detail::ResolvedOperationId
CompileContext::resolve_operation(detail::ResolvedOperation operation) {
  if (!valid_resolved_operation(operation) ||
      m_resolved_operations.size() >= detail::null_resolved_operation_id) {
    throw std::invalid_argument("invalid resolved Flow operation");
  }
  const auto id =
      static_cast<detail::ResolvedOperationId>(m_resolved_operations.size());
  m_resolved_operations.push_back(std::move(operation));
  return id;
}

detail::ResolvedOperation CompileContext::release_resolved_operation(
    detail::ResolvedOperationId operation) {
  if (operation >= m_resolved_operations.size() ||
      !valid_resolved_operation(m_resolved_operations[operation])) {
    throw std::out_of_range("resolved Flow operation is invalid");
  }
  auto &stored = m_resolved_operations[operation];
  auto resolved = std::move(stored);
  stored.external.reset();
  stored.runtime = {};
  return resolved;
}

void CompileContext::clear() noexcept {
  for (auto &entry : m_entries) {
    entry.destroy(entry.value);
    allocator_ref{}.deallocate(entry.value, entry.size, entry.alignment);
  }
  m_entries.clear();
}

CompileContext::Entry *CompileContext::find(std::string_view id) {
  return const_cast<Entry *>(std::as_const(*this).find(id));
}

const CompileContext::Entry *CompileContext::find(std::string_view id) const {
  for (const auto &entry : m_entries) {
    if (entry.id.view() == id) {
      return &entry;
    }
  }
  return nullptr;
}

} // namespace strobe::flow
