#pragma once

#include "strobe/flow/detail/compiler/resolved_ir.hpp"

#include <strobe/core/containers/vector.hpp>

#include <cstdint>

namespace strobe::flow::detail::compiler {

struct LoweredCutoff {
  NodeId node;
  ValueId input;
  ValueId output;
  std::uint32_t source_cutoff;
};

struct CutoffIR {
  ResolvedIR resolved;
  Vector<LoweredCutoff, allocator_ref> cutoffs;
};

} // namespace strobe::flow::detail::compiler
