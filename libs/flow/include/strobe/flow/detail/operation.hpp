#pragma once

#include "strobe/flow/allocator.hpp"
#include "strobe/flow/detail/access_info.hpp"
#include "strobe/flow/detail/operation_kind.hpp"
#include "strobe/flow/lifetime.hpp"
#include "strobe/flow/op_source.hpp"

#include <strobe/core/containers/span.hpp>
#include <strobe/core/containers/vector.hpp>

#include <cstddef>
#include <functional>
#include <optional>

namespace strobe::flow::detail {

struct Operation {
  OperationKind kind = OperationKind::callback;
  OpMetadata metadata;
  std::optional<ExternalOp> external;
  Vector<TypeId, allocator_ref> argument_types;
  Vector<AccessMode, allocator_ref> argument_accesses;
  Vector<AccessInfo, allocator_ref> accesses;
  Vector<std::size_t, allocator_ref> predecessors;
  std::move_only_function<void(Lifetime, span<void *const>)> invoke;
};

} // namespace strobe::flow::detail
