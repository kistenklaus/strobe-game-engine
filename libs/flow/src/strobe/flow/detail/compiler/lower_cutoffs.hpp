#pragma once

#include "strobe/flow/detail/compiler/cutoff_ir.hpp"

#include <stdexcept>
#include <utility>

namespace strobe::flow::detail::compiler {

inline CutoffIR lower_cutoffs(ResolvedIR resolved) {
  CutoffIR result;
  result.resolved = std::move(resolved);
  const auto &values = result.resolved.values;
  result.cutoffs.reserve(values.cutoffs.size());

  for (const auto &cutoff : values.cutoffs) {
    if (cutoff.node >= values.nodes.size() ||
        values.nodes[cutoff.node].kind != NodeKind::cutoff) {
      throw std::logic_error("cutoff node is invalid");
    }

    const auto accesses = values.nodes[cutoff.node].accesses;
    if (accesses.count != 2 ||
        accesses.offset + accesses.count > values.accesses.size()) {
      throw std::logic_error("cutoff accesses are invalid");
    }
    const auto &input = values.accesses[accesses.offset];
    const auto &output = values.accesses[accesses.offset + 1];
    if (input.resource != cutoff.input || input.mode != AccessMode::read ||
        input.input == null_value_id || input.output != null_value_id ||
        output.resource != cutoff.output || output.mode != AccessMode::write ||
        output.input != null_value_id || output.output == null_value_id) {
      throw std::logic_error("cutoff value accesses are inconsistent");
    }
    if (input.input >= values.values.size() ||
        output.output >= values.values.size() ||
        values.values[input.input].resource != cutoff.input ||
        values.values[output.output].resource != cutoff.output ||
        values.values[output.output].producer != cutoff.node) {
      throw std::logic_error("cutoff static values are inconsistent");
    }

    result.cutoffs.push_back(LoweredCutoff{
        .node = cutoff.node,
        .input = input.input,
        .output = output.output,
        .source_cutoff = cutoff.source_cutoff,
    });
  }

  return result;
}

} // namespace strobe::flow::detail::compiler
