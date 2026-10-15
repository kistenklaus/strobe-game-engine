#pragma once

#include "strobe/flow/detail/compiler/storage_ir.hpp"

#include <strobe/core/containers/vector.hpp>

#include <utility>

namespace strobe::flow::detail::compiler {

struct ResourceLifetime {
  StorageClass storage;
  IndexRange definitions;
  IndexRange uses;
  IndexRange last_uses;
  IndexRange activation_sources;
  bool externally_triggered;
  bool may_overlap_generations;
  bool memoized_reconstruction;
  bool preconstructed;
  bool retained_by_feedback;
  bool destroy_at_shutdown;
};

struct LifetimeIR {
  LifetimeIR(StorageIR input, Vector<ResourceLifetime, allocator_ref> resources,
             Vector<NodeId, allocator_ref> definitions,
             Vector<NodeId, allocator_ref> uses,
             Vector<NodeId, allocator_ref> last_uses,
             Vector<ComponentId, allocator_ref> activation_sources)
      : input(std::move(input)), resources(std::move(resources)),
        definitions(std::move(definitions)), uses(std::move(uses)),
        last_uses(std::move(last_uses)),
        activation_sources(std::move(activation_sources)) {}

  StorageIR input;
  Vector<ResourceLifetime, allocator_ref> resources;
  Vector<NodeId, allocator_ref> definitions;
  Vector<NodeId, allocator_ref> uses;
  Vector<NodeId, allocator_ref> last_uses;
  Vector<ComponentId, allocator_ref> activation_sources;
};

} // namespace strobe::flow::detail::compiler
