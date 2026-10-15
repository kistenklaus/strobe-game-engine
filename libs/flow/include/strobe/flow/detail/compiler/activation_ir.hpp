#pragma once

#include "strobe/flow/detail/compiler/component_ir.hpp"

#include <strobe/core/containers/vector.hpp>

#include <utility>

namespace strobe::flow::detail::compiler {

struct ActivationInfo {
  IndexRange feedback_sources;
  bool runs_once;
  bool externally_triggered;
  bool memoized;
  bool contains_yield;
  bool reaches_yield;
};

struct ActivationIR {
  ActivationIR(ComponentIR input,
               Vector<ActivationInfo, allocator_ref> components,
               Vector<ComponentId, allocator_ref> feedback_sources)
      : input(std::move(input)), components(std::move(components)),
        feedback_sources(std::move(feedback_sources)) {}

  ComponentIR input;
  Vector<ActivationInfo, allocator_ref> components;
  Vector<ComponentId, allocator_ref> feedback_sources;
};

} // namespace strobe::flow::detail::compiler
