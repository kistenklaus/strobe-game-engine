#pragma once

#include "strobe/flow/access.hpp"
#include "strobe/flow/allocator.hpp"
#include "strobe/flow/detail/acquire_info.hpp"
#include "strobe/flow/detail/cutoff_info.hpp"
#include "strobe/flow/detail/external_info.hpp"
#include "strobe/flow/detail/if_info.hpp"
#include "strobe/flow/detail/operation.hpp"
#include "strobe/flow/detail/phi_info.hpp"
#include "strobe/flow/detail/projection_info.hpp"
#include "strobe/flow/detail/resource_info.hpp"

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <strobe/core/containers/span.hpp>
#include <strobe/core/containers/vector.hpp>
#include <tuple>
#include <type_traits>
#include <utility>

namespace strobe::flow::detail {

template <std::size_t Count>
using callback_access_indices =
    std::make_index_sequence<Count == 0 ? 0 : Count - 1>;

class OpStream {
public:
  OpStream() = default;
  OpStream(const OpStream &) = delete;
  OpStream &operator=(const OpStream &) = delete;
  OpStream(OpStream &&) = delete;
  OpStream &operator=(OpStream &&) = delete;
  ~OpStream() = default;

  template <typename T> Resource<T> resource(std::string_view label = {}) {
    static_assert(std::is_object_v<T> && !std::is_array_v<T> &&
                  !std::is_const_v<T> && !std::is_volatile_v<T> &&
                  std::is_nothrow_destructible_v<T>);
    if (m_resources.size() >= std::numeric_limits<ResourceId>::max()) {
      throw std::length_error("resource id space exhausted");
    }
    const auto id = static_cast<ResourceId>(m_resources.size());
    m_resources.push_back(ResourceInfo{
        .type = type_id<T>(),
        .type_name = String<allocator_ref>(flow::type_name<T>(), allocator_ref{}),
        .label = String<allocator_ref>(label, allocator_ref{}),
        .size = sizeof(T),
        .alignment = alignof(T),
        .destroy =
            [](void *storage) noexcept {
              std::destroy_at(static_cast<T *>(storage));
            },
        .construction = std::nullopt,
        .phi = std::nullopt,
        .cutoff = std::nullopt,
        .external = std::nullopt,
        .projection = std::nullopt,
    });
    m_defined.push_back(false);
    return Resource<T>(id);
  }

  template <typename T> Resource<T> phi(const Resource<T> &initial) {
    validate(initial);
    const auto &initial_info = m_resources[initial.m_id];
    if (!initial_info.construction && !initial_info.phi &&
        !initial_info.cutoff && !initial_info.external) {
      throw std::logic_error(
          "phi initial resource must be defined before recording the phi");
    }
    if (!m_defined[initial.m_id]) {
      throw std::logic_error("phi initial resource has an undefined value");
    }
    if (m_resources.size() >= std::numeric_limits<ResourceId>::max()) {
      throw std::length_error("resource id space exhausted");
    }
    const auto id = static_cast<ResourceId>(m_resources.size());
    const auto phi_index = m_phis.size();
    m_resources.push_back(ResourceInfo{
        .type = type_id<T>(),
        .type_name = String<allocator_ref>(flow::type_name<T>(), allocator_ref{}),
        .label = String<allocator_ref>(allocator_ref{}),
        .size = 0,
        .alignment = 1,
        .destroy = nullptr,
        .construction = std::nullopt,
        .phi = phi_index,
        .cutoff = std::nullopt,
        .external = std::nullopt,
        .projection = std::nullopt,
    });
    m_phis.push_back(PhiInfo{
        .output = id,
        .initial = initial.m_id,
        .feedback = std::nullopt,
        .condition = std::nullopt,
        .position = m_ops.size(),
        .feedback_position = std::nullopt,
        .max_in_flight = 2,
    });
    m_defined.push_back(true);
    return Resource<T>(id);
  }

  template <typename T> Resource<T> external(std::string_view name) {
    static_assert(std::is_object_v<T> && !std::is_array_v<T> &&
                  !std::is_const_v<T> && !std::is_volatile_v<T>);
    if (name.empty()) {
      throw std::invalid_argument("external resource name cannot be empty");
    }
    const auto existing = std::find_if(m_externals.begin(), m_externals.end(),
                                       [&](const ExternalInfo &external) {
                                         return external.name.view() == name;
                                       });
    if (existing != m_externals.end()) {
      const auto resource = existing->resource;
      if (resource >= m_resources.size() ||
          m_resources[resource].type != type_id<T>()) {
        throw std::invalid_argument(
            "external resource name has a different type");
      }
      return Resource<T>(resource);
    }
    if (m_resources.size() >= std::numeric_limits<ResourceId>::max()) {
      throw std::length_error("resource id space exhausted");
    }

    const auto id = static_cast<ResourceId>(m_resources.size());
    const auto external_index = m_externals.size();
    m_resources.push_back(ResourceInfo{
        .type = type_id<T>(),
        .type_name = String<allocator_ref>(flow::type_name<T>(), allocator_ref{}),
        .label = String<allocator_ref>(name, allocator_ref{}),
        .size = sizeof(T),
        .alignment = alignof(T),
        .destroy = nullptr,
        .construction = std::nullopt,
        .phi = std::nullopt,
        .cutoff = std::nullopt,
        .external = external_index,
        .projection = std::nullopt,
    });
    m_defined.push_back(true);
    m_externals.push_back(ExternalInfo{
        .resource = id,
        .name = String<allocator_ref>(name, allocator_ref{}),
    });
    return Resource<T>(id);
  }

  template <typename T, typename Owner, typename Projector>
    requires std::invocable<Projector &, Owner &> &&
             std::same_as<std::invoke_result_t<Projector &, Owner &>, T *>
  Resource<T> project(const Resource<Owner> &owner, std::string_view name,
                      ProjectionKind kind, Projector &&projector) {
    validate(owner);
    if (name.empty()) {
      throw std::invalid_argument("projection name cannot be empty");
    }
    const auto existing =
        std::find_if(m_projections.begin(), m_projections.end(),
                     [&](const ProjectionInfo &projection) {
                       return projection.owner == owner.m_id &&
                              projection.name.view() == name;
                     });
    if (existing != m_projections.end()) {
      const auto resource = existing->resource;
      if (m_resources[resource].type != type_id<T>() ||
          existing->kind != kind) {
        throw std::invalid_argument(
            "projection name has a different type or kind");
      }
      return Resource<T>(resource);
    }
    if (m_resources.size() >= std::numeric_limits<ResourceId>::max()) {
      throw std::length_error("resource id space exhausted");
    }
    const auto id = static_cast<ResourceId>(m_resources.size());
    const auto projection_index = m_projections.size();
    m_resources.push_back(ResourceInfo{
        .type = type_id<T>(),
        .type_name = String<allocator_ref>(flow::type_name<T>(), allocator_ref{}),
        .label = String<allocator_ref>(name, allocator_ref{}),
        .size = 0,
        .alignment = 1,
        .destroy = nullptr,
        .construction = std::nullopt,
        .phi = std::nullopt,
        .cutoff = std::nullopt,
        .external = std::nullopt,
        .projection = projection_index,
    });
    m_defined.push_back(true);
    m_projections.push_back(ProjectionInfo{
        .resource = id,
        .owner = owner.m_id,
        .kind = kind,
        .name = String<allocator_ref>(name, allocator_ref{}),
        .resolve = [projector = std::forward<Projector>(projector)](
                       void *value) -> void * {
          return std::invoke(projector, *static_cast<Owner *>(value));
        },
        .external = std::nullopt,
    });
    return Resource<T>(id);
  }

  template <typename T, typename Owner>
  Resource<T> project(const Resource<Owner> &owner, std::string_view name,
                      ProjectionKind kind, ExternalProjector projector) {
    validate(owner);
    if (name.empty()) {
      throw std::invalid_argument("projection name cannot be empty");
    }
    validate_external_op(projector.source);
    const auto existing =
        std::find_if(m_projections.begin(), m_projections.end(),
                     [&](const ProjectionInfo &projection) {
                       return projection.owner == owner.m_id &&
                              projection.name.view() == name;
                     });
    if (existing != m_projections.end()) {
      const auto resource = existing->resource;
      if (m_resources[resource].type != type_id<T>() ||
          existing->kind != kind) {
        throw std::invalid_argument(
            "projection name has a different type or kind");
      }
      return Resource<T>(resource);
    }
    if (m_resources.size() >= std::numeric_limits<ResourceId>::max()) {
      throw std::length_error("resource id space exhausted");
    }
    const auto id = static_cast<ResourceId>(m_resources.size());
    const auto projection_index = m_projections.size();
    m_resources.push_back(ResourceInfo{
        .type = type_id<T>(),
        .type_name = String<allocator_ref>(flow::type_name<T>(), allocator_ref{}),
        .label = String<allocator_ref>(name, allocator_ref{}),
        .size = 0,
        .alignment = 1,
        .destroy = nullptr,
        .construction = std::nullopt,
        .phi = std::nullopt,
        .cutoff = std::nullopt,
        .external = std::nullopt,
        .projection = projection_index,
    });
    m_defined.push_back(true);
    m_projections.push_back(ProjectionInfo{
        .resource = id,
        .owner = owner.m_id,
        .kind = kind,
        .name = String<allocator_ref>(name, allocator_ref{}),
        .resolve = {},
        .external = std::move(projector.source),
    });
    return Resource<T>(id);
  }

  template <typename T>
    requires std::copy_constructible<T> &&
             requires(const T &left, const T &right) {
               { left == right } -> std::convertible_to<bool>;
             }
  Resource<T> cutoff(const Resource<T> &input) {
    validate(input);
    if (!m_defined[input.m_id]) {
      throw std::logic_error("cutoff input has an undefined value");
    }
    if (m_resources.size() >= std::numeric_limits<ResourceId>::max()) {
      throw std::length_error("resource id space exhausted");
    }

    const auto output = static_cast<ResourceId>(m_resources.size());
    const auto cutoff_index = m_cutoffs.size();
    m_resources.push_back(ResourceInfo{
        .type = type_id<T>(),
        .type_name = String<allocator_ref>(flow::type_name<T>(), allocator_ref{}),
        .label = String<allocator_ref>(allocator_ref{}),
        .size = 0,
        .alignment = 1,
        .destroy = nullptr,
        .construction = std::nullopt,
        .phi = std::nullopt,
        .cutoff = cutoff_index,
        .external = std::nullopt,
        .projection = std::nullopt,
    });
    m_defined.push_back(true);

    Operation operation;
    operation.kind = OperationKind::cutoff;
    operation.accesses.push_back({input.m_id, AccessMode::read});
    operation.accesses.push_back({output, AccessMode::write});
    operation.invoke = [](Lifetime, span<void *const> storage) {
      validate_storage(storage, 0);
    };
    const auto operation_index = append(std::move(operation));

    m_cutoffs.push_back(CutoffInfo{
        .input = input.m_id,
        .output = output,
        .operation = operation_index,
        .equal =
            [](const void *left, const void *right) {
              return *static_cast<const T *>(left) ==
                     *static_cast<const T *>(right);
            },
        .copy_construct =
            [](void *destination, const void *source) {
              std::construct_at(static_cast<T *>(destination),
                                *static_cast<const T *>(source));
            },
        .destroy =
            [](void *value) noexcept {
              std::destroy_at(static_cast<T *>(value));
            },
        .size = sizeof(T),
        .alignment = alignof(T),
    });
    return Resource<T>(output);
  }

  template <typename T>
  void feedback(const Resource<T> &phi, const Resource<T> &value,
                const FeedbackOptions &options = {}) {
    validate(phi);
    validate(value);
    const auto &phi_resource = m_resources[phi.m_id];
    if (!phi_resource.phi) {
      throw std::invalid_argument("feedback target is not a phi resource");
    }
    const auto &value_info = m_resources[value.m_id];
    if (!value_info.construction && !value_info.phi && !value_info.cutoff &&
        !value_info.external) {
      throw std::logic_error(
          "feedback resource must be defined before recording feedback");
    }
    if (!m_defined[value.m_id]) {
      throw std::logic_error("feedback resource has an undefined value");
    }
    auto &info = m_phis[*phi_resource.phi];
    if (info.feedback) {
      throw std::logic_error("phi already has a feedback resource");
    }
    if (options.max_in_flight == 0) {
      throw std::invalid_argument(
          "feedback max_in_flight must be greater than zero");
    }
    info.feedback = value.m_id;
    info.condition = std::nullopt;
    info.feedback_position = m_ops.size();
    info.max_in_flight = options.max_in_flight;
  }

  template <typename Body>
    requires std::invocable<Body &>
  void while_(const Resource<bool> &condition, Body &&body) {
    validate(condition);
    if (!m_defined[condition.m_id]) {
      throw std::logic_error("while condition has an undefined value");
    }
    const auto loop = phi(condition);
    std::invoke(body);
    auto &info = m_phis[*m_resources[loop.m_id].phi];
    info.feedback = condition.m_id;
    info.condition = condition.m_id;
    info.feedback_position = m_ops.size();
    info.max_in_flight = 1;
  }

  template <typename Then, typename Else>
    requires std::invocable<Then &> && std::invocable<Else &>
  void if_(const Resource<bool> &condition, Then &&then_body,
           Else &&else_body) {
    validate(condition);
    if (!m_defined[condition.m_id]) {
      throw std::logic_error("if condition has an undefined value");
    }
    const auto begin = m_ops.size();
    std::invoke(then_body);
    const auto alternative = m_ops.size();
    std::invoke(else_body);
    const auto end = m_ops.size();
    if (begin == end) {
      throw std::logic_error("if requires at least one recorded operation");
    }
    for (std::size_t operation = begin; operation < end; ++operation) {
      if (m_ops[operation].kind == OperationKind::acquire ||
          m_ops[operation].kind == OperationKind::yield) {
        throw std::logic_error("if body cannot contain a submission boundary");
      }
      if (m_ops[operation].kind == OperationKind::construct ||
          m_ops[operation].kind == OperationKind::discard) {
        throw std::logic_error(
            "if body cannot construct or discard a resource");
      }
      if (std::any_of(m_ops[operation].accesses.begin(),
                      m_ops[operation].accesses.end(),
                      [](const AccessInfo &access) {
                        return access.mode == AccessMode::consume;
                      })) {
        throw std::logic_error("if body cannot consume a resource");
      }
    }
    m_ifs.push_back(IfInfo{
        .condition = condition.m_id,
        .begin = begin,
        .alternative = alternative,
        .end = end,
    });
  }

  template <typename T, typename... Args>
    requires std::is_constructible_v<T, std::decay_t<Args>...>
  std::size_t construct(const Resource<T> &resource, Args &&...args) {
    return construct_value(resource, OpMetadata{}, std::forward<Args>(args)...);
  }

  template <typename T, typename... Args>
  std::size_t construct_with_metadata(const Resource<T> &resource,
                                      OpMetadata metadata, Args &&...args) {
    if constexpr (std::is_constructible_v<T, std::decay_t<Args>...>) {
      return construct_value(resource, std::move(metadata),
                             std::forward<Args>(args)...);
    } else if constexpr (sizeof...(Args) == 0) {
      static_assert(sizeof...(Args) > 0,
                    "construct requires values or a callback");
      return 0;
    } else {
      validate_construction(resource);
      auto values =
          std::tuple<std::decay_t<Args>...>(std::forward<Args>(args)...);
      const auto index = append_construction<T>(
          resource, std::move(metadata), std::move(values),
          callback_access_indices<sizeof...(Args)>{});
      m_resources[resource.m_id].construction = index;
      update_defined_resources(
          {m_ops[index].accesses.data(), m_ops[index].accesses.size()});
      return index;
    }
  }

private:
  template <typename T, typename... Args>
  std::size_t construct_value(const Resource<T> &resource, OpMetadata metadata,
                              Args &&...args) {
    validate_construction(resource);
    Operation operation;
    operation.kind = OperationKind::construct;
    operation.metadata = std::move(metadata);
    operation.accesses.push_back({resource.m_id, AccessMode::write});
    operation.argument_types.push_back(type_id<T>());
    operation.argument_accesses.push_back(AccessMode::write);
    operation.argument_elements.push_back(0);
    operation.argument_groups.push_back({0, 1, 0, 1});
    operation.invoke = [values = std::tuple<std::decay_t<Args>...>(
                            std::forward<Args>(args)...)](
                           Lifetime, span<void *const> storage) mutable {
      validate_storage(storage, 1);
      std::apply(
          [&](auto &...values) {
            std::construct_at(static_cast<T *>(storage[0]),
                              std::move(values)...);
          },
          values);
    };
    const auto index = append(std::move(operation));
    m_resources[resource.m_id].construction = index;
    update_defined_resources(
        {m_ops[index].accesses.data(), m_ops[index].accesses.size()});
    return index;
  }

public:
  template <typename T, typename... Args>
    requires(sizeof...(Args) > 0 &&
             !std::is_constructible_v<T, std::decay_t<Args>...>)
  std::size_t construct(const Resource<T> &resource, Args &&...args) {
    validate_construction(resource);
    auto values =
        std::tuple<std::decay_t<Args>...>(std::forward<Args>(args)...);
    const auto index =
        append_construction<T>(resource, OpMetadata{}, std::move(values),
                               callback_access_indices<sizeof...(Args)>{});
    m_resources[resource.m_id].construction = index;
    update_defined_resources(
        {m_ops[index].accesses.data(), m_ops[index].accesses.size()});
    return index;
  }

  template <typename T, typename... Accesses>
  std::size_t construct(const Resource<T> &resource, ExternalOp source,
                        Accesses &&...accesses) {
    static_assert((is_recorded_access<Accesses> && ...),
                  "external construct inputs must be resource accesses");
    static_assert(valid_access_order<Accesses...>(),
                  "serialization accesses must be trailing");
    validate_construction(resource);
    validate_external_op(source);
    Operation operation;
    operation.kind = OperationKind::construct;
    operation.metadata = std::move(source.metadata);
    source.metadata = {};
    operation.external = std::move(source);
    operation.accesses.push_back({resource.m_id, AccessMode::write});
    operation.argument_types.push_back(type_id<T>());
    operation.argument_accesses.push_back(AccessMode::write);
    operation.argument_elements.push_back(0);
    operation.argument_groups.push_back({0, 1, 0, 1});
    (append_external_access(operation, std::forward<Accesses>(accesses)), ...);
    validate_unique_accesses(
        {operation.accesses.data(), operation.accesses.size()});
    const auto index = append(std::move(operation));
    m_resources[resource.m_id].construction = index;
    update_defined_resources(
        {m_ops[index].accesses.data(), m_ops[index].accesses.size()});
    return index;
  }

  template <typename T, typename... Accesses>
  std::size_t construct(const Resource<T> &resource, IntrinsicOp source,
                        Accesses &&...accesses) {
    static_assert((is_recorded_access<Accesses> && ...),
                  "intrinsic construct inputs must be resource accesses");
    static_assert(valid_access_order<Accesses...>(),
                  "serialization accesses must be trailing");
    validate_construction(resource);
    Operation operation;
    operation.kind = OperationKind::construct;
    operation.intrinsic = std::move(source);
    operation.accesses.push_back({resource.m_id, AccessMode::write});
    operation.argument_types.push_back(type_id<T>());
    operation.argument_accesses.push_back(AccessMode::write);
    operation.argument_elements.push_back(0);
    operation.argument_groups.push_back({0, 1, 0, 1});
    (append_external_access(operation, std::forward<Accesses>(accesses)), ...);
    validate_unique_accesses(
        {operation.accesses.data(), operation.accesses.size()});
    const auto index = append(std::move(operation));
    m_resources[resource.m_id].construction = index;
    update_defined_resources(
        {m_ops[index].accesses.data(), m_ops[index].accesses.size()});
    return index;
  }

  template <typename T> std::size_t discard(const Resource<T> &resource) {
    validate(resource);
    if (m_resources[resource.m_id].external) {
      throw std::logic_error("external resources cannot be discarded");
    }
    if (!m_defined[resource.m_id]) {
      throw std::logic_error("cannot discard an undefined resource value");
    }
    Operation operation;
    operation.kind = OperationKind::discard;
    operation.accesses.push_back({resource.m_id, AccessMode::write});
    operation.invoke = [](Lifetime, span<void *const> storage) {
      validate_storage(storage, 0);
    };
    const auto index = append(std::move(operation));
    m_defined[resource.m_id] = false;
    return index;
  }

  void validate() const;

  template <typename... Args>
    requires(sizeof...(Args) > 0)
  std::size_t op(Args &&...args) {
    auto values =
        std::tuple<std::decay_t<Args>...>(std::forward<Args>(args)...);
    return append(OpMetadata{}, std::move(values),
                  callback_access_indices<sizeof...(Args)>{});
  }

  template <typename... Args>
    requires(sizeof...(Args) > 0)
  std::size_t op_with_metadata(OpMetadata metadata, Args &&...args) {
    auto values =
        std::tuple<std::decay_t<Args>...>(std::forward<Args>(args)...);
    return append(std::move(metadata), std::move(values),
                  callback_access_indices<sizeof...(Args)>{});
  }

  template <typename... Accesses>
  std::size_t op(ExternalOp source, Accesses &&...accesses) {
    static_assert((is_recorded_access<Accesses> && ...),
                  "external op arguments must be resource accesses");
    static_assert(valid_access_order<Accesses...>(),
                  "serialization accesses must be trailing");
    validate_external_op(source);
    Operation operation;
    operation.metadata = std::move(source.metadata);
    source.metadata = {};
    operation.external = std::move(source);
    (append_external_access(operation, std::forward<Accesses>(accesses)), ...);
    validate_unique_accesses(
        {operation.accesses.data(), operation.accesses.size()});
    const auto index = append(std::move(operation));
    update_defined_resources(
        {m_ops[index].accesses.data(), m_ops[index].accesses.size()});
    return index;
  }

  template <typename... Accesses>
  std::size_t op(IntrinsicOp source, Accesses &&...accesses) {
    static_assert((is_recorded_access<Accesses> && ...),
                  "intrinsic op arguments must be resource accesses");
    static_assert(valid_access_order<Accesses...>(),
                  "serialization accesses must be trailing");
    Operation operation;
    operation.intrinsic = std::move(source);
    (append_external_access(operation, std::forward<Accesses>(accesses)), ...);
    validate_unique_accesses(
        {operation.accesses.data(), operation.accesses.size()});
    const auto index = append(std::move(operation));
    update_defined_resources(
        {m_ops[index].accesses.data(), m_ops[index].accesses.size()});
    return index;
  }

  std::size_t yield();

  std::size_t acquire(std::string_view name);

  std::size_t size() const noexcept;

  span<const std::size_t> predecessors(std::size_t index) const;

  const Vector<Operation, allocator_ref> &ops() const noexcept;

  const Vector<ResourceInfo, allocator_ref> &resources() const noexcept;

  const Vector<PhiInfo, allocator_ref> &phis() const noexcept;

  const Vector<CutoffInfo, allocator_ref> &cutoffs() const noexcept;

  const Vector<AcquireInfo, allocator_ref> &acquires() const noexcept;
  const Vector<IfInfo, allocator_ref> &ifs() const noexcept;

  std::move_only_function<bool(const void *, const void *) const>
  release_cutoff_equal(std::size_t cutoff);

  std::move_only_function<void(void *, const void *) const>
  release_cutoff_copy(std::size_t cutoff);

  const Vector<ExternalInfo, allocator_ref> &externals() const noexcept;
  const Vector<ProjectionInfo, allocator_ref> &projections() const noexcept {
    return m_projections;
  }
  std::move_only_function<void *(void *) const>
  release_projection(std::size_t projection) {
    if (projection >= m_projections.size()) {
      throw std::out_of_range("projection index is out of range");
    }
    return std::move(m_projections[projection].resolve);
  }

  std::move_only_function<void(Lifetime, span<void *const>)>
  release_callback(std::size_t operation);

private:
  void validate_resource(std::size_t index, const ResourceInfo &resource) const;

  void validate_phi(std::size_t index, const PhiInfo &phi) const;

  void validate_cutoff(std::size_t index, const CutoffInfo &cutoff) const;

  void validate_external(std::size_t index, const ExternalInfo &external) const;

  void validate_operation(std::size_t index, const Operation &operation) const;

  template <typename T>
  void validate_construction(const Resource<T> &resource) const {
    validate(resource);
    const auto &info = m_resources[resource.m_id];
    if (info.phi || info.cutoff || info.external) {
      throw std::logic_error("non-owned resources cannot be constructed");
    }
    if (info.construction) {
      throw std::logic_error("resource already has a construction operation");
    }
  }

  template <typename T> void validate(const Resource<T> &resource) const {
    if (resource.m_id >= m_resources.size()) {
      throw std::invalid_argument("resource does not belong to this stream");
    }
  }

  static void validate_storage(span<void *const> storage, std::size_t count);

  static void validate_unique_accesses(span<const AccessInfo> accesses);

  static void validate_external_op(const ExternalOp &source);

  void update_defined_resources(span<const AccessInfo> accesses) {
    for (const auto &access : accesses) {
      if (access.purpose != AccessPurpose::argument ||
          access.mode == AccessMode::read) {
        continue;
      }
      m_defined[access.resource] = access.mode != AccessMode::consume;
    }
  }

  template <typename T, AccessMode Mode>
  void append_external_access(Operation &operation, Access<T, Mode> access) {
    const auto offset =
        static_cast<std::uint32_t>(operation.argument_types.size());
    const auto element_offset =
        static_cast<std::uint32_t>(operation.argument_elements.size());
    if (access.m_resource) {
      operation.accesses.push_back(describe(access));
      operation.argument_types.push_back(type_id<T>());
      operation.argument_accesses.push_back(Mode);
      operation.argument_elements.push_back(offset);
      operation.argument_groups.push_back({offset, 1, element_offset, 1});
    } else {
      operation.argument_elements.push_back(null_argument_element);
      operation.argument_groups.push_back({offset, 0, element_offset, 1});
    }
  }

  template <typename T, AccessMode Mode>
  void append_external_access(Operation &operation,
                              AccessRange<T, Mode> access) {
    const auto offset =
        static_cast<std::uint32_t>(operation.argument_types.size());
    const auto element_offset =
        static_cast<std::uint32_t>(operation.argument_elements.size());
    for (const auto &resource : access.m_resources) {
      if (resource) {
        const auto argument =
            static_cast<std::uint32_t>(operation.argument_types.size());
        operation.accesses.push_back(describe(Access<T, Mode>(resource)));
        operation.argument_types.push_back(type_id<T>());
        operation.argument_accesses.push_back(Mode);
        operation.argument_elements.push_back(argument);
      } else {
        operation.argument_elements.push_back(null_argument_element);
      }
    }
    operation.argument_groups.push_back(
        {offset,
         static_cast<std::uint32_t>(operation.argument_types.size() - offset),
         element_offset,
         static_cast<std::uint32_t>(access.m_resources.size())});
  }

  template <typename T>
  void append_external_access(Operation &operation,
                              SerializationAccess<T> access) {
    if (access.m_resource) {
      operation.accesses.push_back(describe(access));
    }
  }

  template <typename T, AccessMode Mode>
  AccessInfo describe(const Access<T, Mode> &access) const {
    validate(access.m_resource);
    const auto &resource = m_resources[access.m_resource.m_id];
    if (!resource.construction && !resource.phi && !resource.cutoff &&
        !resource.external && !resource.projection) {
      throw std::logic_error(
          "resource must be defined before recording accesses");
    }
    if constexpr (Mode != AccessMode::write) {
      if (!m_defined[access.m_resource.m_id]) {
        throw std::logic_error("cannot read an undefined resource value");
      }
    }
    return {access.m_resource.m_id, Mode, AccessPurpose::argument};
  }

  template <typename T>
  AccessInfo describe(const SerializationAccess<T> &access) const {
    validate(access.m_resource);
    const auto &resource = m_resources[access.m_resource.m_id];
    if (!resource.construction && !resource.phi && !resource.cutoff &&
        !resource.external && !resource.projection) {
      throw std::logic_error(
          "resource must be defined before recording serialization");
    }
    if (!m_defined[access.m_resource.m_id]) {
      throw std::logic_error("cannot serialize an undefined resource value");
    }
    return {access.m_resource.m_id, AccessMode::read_write,
            AccessPurpose::serialization};
  }

  template <typename... Accesses> static consteval bool valid_access_order() {
    if constexpr (sizeof...(Accesses) == 0) {
      return true;
    } else {
      bool saw_serialization = false;
      bool valid = true;
      ((is_serialization_access<Accesses>
            ? saw_serialization = true
            : valid = valid && !saw_serialization),
       ...);
      return valid;
    }
  }

  template <typename Tuple, std::size_t... I>
  static consteval std::size_t argument_count(std::index_sequence<I...>) {
    static_assert(
        (is_any_access<decltype(std::get<I>(std::declval<Tuple &>()))> && ...),
        "operation inputs must be resource accesses");
    static_assert(
        valid_access_order<decltype(std::get<I>(std::declval<Tuple &>()))...>(),
        "serialization accesses must be trailing");
    return (
        std::size_t{0} + ... +
        (is_access<decltype(std::get<I>(std::declval<Tuple &>()))> ? 1u : 0u));
  }

  template <typename Tuple, std::size_t... I>
  std::size_t append(OpMetadata metadata, Tuple values,
                     std::index_sequence<I...>) {
    constexpr auto count = argument_count<Tuple>(std::index_sequence<I...>{});
    return append_callback(std::move(metadata), std::move(values),
                           std::make_index_sequence<count>{},
                           std::index_sequence<I...>{});
  }

  template <typename Tuple, std::size_t... A, std::size_t... I>
  std::size_t append_callback(OpMetadata metadata, Tuple values,
                              std::index_sequence<A...>,
                              std::index_sequence<I...>) {
    using Callback = decltype(std::get<sizeof...(I)>(values));
    constexpr bool invocable =
        std::is_invocable_v<Callback &,
                            decltype(std::get<A>(values).get(nullptr))...>;
    static_assert(invocable,
                  "op callback arguments must match its resource accesses");
    if constexpr (invocable) {
      static_assert(
          std::is_same_v<void, std::invoke_result_t<
                                   Callback &, decltype(std::get<A>(values).get(
                                                   nullptr))...>>,
          "op callbacks must return void");
    }

    Operation operation;
    operation.metadata = std::move(metadata);
    (operation.accesses.push_back(describe(std::get<I>(values))), ...);
    ((operation.argument_types.push_back(
          std::remove_cvref_t<decltype(std::get<A>(values))>::argument_type),
      operation.argument_accesses.push_back(operation.accesses[A].mode),
      operation.argument_elements.push_back(static_cast<std::uint32_t>(A)),
      operation.argument_groups.push_back({static_cast<std::uint32_t>(A), 1,
                                           static_cast<std::uint32_t>(A), 1})),
     ...);
    validate_unique_accesses(
        {operation.accesses.data(), operation.accesses.size()});
    operation.invoke = [values = std::move(values)](
                           Lifetime, span<void *const> storage) mutable {
      validate_storage(storage, sizeof...(A));
      std::invoke(std::get<sizeof...(I)>(values),
                  std::get<A>(values).get(storage[A])...);
    };
    const auto index = append(std::move(operation));
    update_defined_resources(
        {m_ops[index].accesses.data(), m_ops[index].accesses.size()});
    return index;
  }

  template <typename T, typename Tuple, std::size_t... I>
  std::size_t append_construction(const Resource<T> &resource,
                                  OpMetadata metadata, Tuple values,
                                  std::index_sequence<I...>) {
    constexpr auto count = argument_count<Tuple>(std::index_sequence<I...>{});
    return append_construction_callback<T>(
        resource, std::move(metadata), std::move(values),
        std::make_index_sequence<count>{}, std::index_sequence<I...>{});
  }

  template <typename T, typename Tuple, std::size_t... A, std::size_t... I>
  std::size_t append_construction_callback(const Resource<T> &resource,
                                           OpMetadata metadata, Tuple values,
                                           std::index_sequence<A...>,
                                           std::index_sequence<I...>) {
    using Callback = decltype(std::get<sizeof...(I)>(values));
    constexpr bool lifetime_invocable =
        std::is_invocable_v<Callback &, T *, Lifetime,
                            decltype(std::get<A>(values).get(nullptr))...>;
    constexpr bool legacy_invocable =
        std::is_invocable_v<Callback &, T *,
                            decltype(std::get<A>(values).get(nullptr))...>;
    static_assert(
        lifetime_invocable || legacy_invocable,
        "construct callback must accept T*, optionally Lifetime, followed by "
        "its inputs");
    if constexpr (lifetime_invocable) {
      static_assert(
          std::is_same_v<void,
                         std::invoke_result_t<Callback &, T *, Lifetime,
                                              decltype(std::get<A>(values).get(
                                                  nullptr))...>>,
          "construct callbacks must return void");
    } else if constexpr (legacy_invocable) {
      static_assert(
          std::is_same_v<void,
                         std::invoke_result_t<Callback &, T *,
                                              decltype(std::get<A>(values).get(
                                                  nullptr))...>>,
          "construct callbacks must return void");
    }

    Operation operation;
    operation.kind = OperationKind::construct;
    operation.metadata = std::move(metadata);
    operation.accesses.push_back({resource.m_id, AccessMode::write});
    operation.argument_types.push_back(type_id<T>());
    operation.argument_accesses.push_back(AccessMode::write);
    operation.argument_elements.push_back(0);
    operation.argument_groups.push_back({0, 1, 0, 1});
    (operation.accesses.push_back(describe(std::get<I>(values))), ...);
    ((operation.argument_types.push_back(
          std::remove_cvref_t<decltype(std::get<A>(values))>::argument_type),
      operation.argument_accesses.push_back(operation.accesses[A + 1].mode),
      operation.argument_elements.push_back(static_cast<std::uint32_t>(A + 1)),
      operation.argument_groups.push_back({static_cast<std::uint32_t>(A + 1), 1,
                                           static_cast<std::uint32_t>(A + 1),
                                           1})),
     ...);
    validate_unique_accesses(
        {operation.accesses.data(), operation.accesses.size()});
    operation.invoke =
        [values = std::move(values)](Lifetime lifetime,
                                     span<void *const> storage) mutable {
          validate_storage(storage, sizeof...(A) + 1);
          if constexpr (lifetime_invocable) {
            std::invoke(std::get<sizeof...(I)>(values),
                        static_cast<T *>(storage[0]), lifetime,
                        std::get<A>(values).get(storage[A + 1])...);
          } else {
            std::invoke(std::get<sizeof...(I)>(values),
                        static_cast<T *>(storage[0]),
                        std::get<A>(values).get(storage[A + 1])...);
          }
        };
    const auto index = append(std::move(operation));
    update_defined_resources(
        {m_ops[index].accesses.data(), m_ops[index].accesses.size()});
    return index;
  }

  std::size_t append(Operation operation);

  Vector<ResourceInfo, allocator_ref> m_resources;
  Vector<std::uint8_t, allocator_ref> m_defined;
  Vector<PhiInfo, allocator_ref> m_phis;
  Vector<CutoffInfo, allocator_ref> m_cutoffs;
  Vector<ExternalInfo, allocator_ref> m_externals;
  Vector<ProjectionInfo, allocator_ref> m_projections;
  Vector<AcquireInfo, allocator_ref> m_acquires;
  Vector<IfInfo, allocator_ref> m_ifs;
  Vector<Operation, allocator_ref> m_ops;
};

String<allocator_ref> to_pretty(const OpStream &stream);

} // namespace strobe::flow::detail
