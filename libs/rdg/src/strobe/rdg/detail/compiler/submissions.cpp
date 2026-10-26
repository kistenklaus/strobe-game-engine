#include "compiler.hpp"
#include "util.hpp"

#include <algorithm>

namespace strobe::rdg::detail {

void collect_submissions(flow::detail::compiler::NormalizedIR &ir,
                         flow::CompileContext &context) {
  auto &submissions = compiler_state(context).submissions;
  for (const auto &node : ir.nodes) {
    if (!intrinsic(node, "submit") || node.accesses.count == 0) {
      continue;
    }
    const auto &command = ir.accesses[node.accesses.offset];
    submissions.push_back({
        .command_buffer = command.resource,
        .position = node.source_operation,
    });
  }
  std::ranges::sort(submissions, {}, &SubmitMarker::position);
}

} // namespace strobe::rdg::detail
