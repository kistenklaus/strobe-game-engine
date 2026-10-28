#include "compiler.hpp"
#include "util.hpp"

#include "strobe/rhi/objects/command_buffer.hpp"
#include "strobe/rhi/objects/command_pool.hpp"
#include "strobe/rhi/objects/timepoint.hpp"

#include <algorithm>
#include <stdexcept>

namespace strobe::rdg::detail {

void collect_submissions(flow::detail::compiler::NormalizedIR &ir,
                         flow::CompileContext &context) {
  auto &submissions = compiler_state(context).submissions;
  for (const auto &node : ir.nodes) {
    if (!intrinsic(node, "submit")) {
      continue;
    }
    const auto accesses = ir.accesses_of(node);
    if (accesses.size() != 3 || node.argument_count != 2) {
      throw std::logic_error("submit intrinsic contract is invalid");
    }
    const auto &timepoint = accesses[0];
    const auto &command = accesses[1];
    const auto &pool = accesses[2];
    if (timepoint.resource >= ir.resources.size() ||
        ir.resources[timepoint.resource].type !=
            flow::type_id<rhi::Timepoint>() ||
        timepoint.mode != flow::AccessMode::write ||
        timepoint.purpose != flow::AccessPurpose::argument ||
        command.resource >= ir.resources.size() ||
        ir.resources[command.resource].type !=
            flow::type_id<rhi::CommandBuffer>() ||
        command.mode != flow::AccessMode::consume ||
        command.purpose != flow::AccessPurpose::argument ||
        pool.resource >= ir.resources.size() ||
        ir.resources[pool.resource].type != flow::type_id<rhi::CommandPool>() ||
        pool.purpose != flow::AccessPurpose::serialization) {
      throw std::logic_error("submit intrinsic accesses are invalid");
    }
    submissions.push_back({
        .command_buffer = command.resource,
        .position = node.source_operation,
    });
  }
  std::ranges::sort(submissions, {}, &SubmitMarker::position);
}

} // namespace strobe::rdg::detail
