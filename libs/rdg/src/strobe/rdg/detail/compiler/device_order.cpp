#include "compiler.hpp"
#include "util.hpp"

#include <algorithm>

namespace strobe::rdg::detail {

void derive_device_order(flow::detail::compiler::NormalizedIR &,
                         flow::CompileContext &context) {
  auto &state = compiler_state(context);

  for (auto &access : state.image_accesses) {
    access.submit_position = submission_for(
        state.submissions, access.command_buffer, access.position);
  }
  for (auto &access : state.buffer_accesses) {
    access.submit_position = submission_for(
        state.submissions, access.command_buffer, access.position);
  }

  const auto device_order = [](const auto &left, const auto &right) {
    if (left.submit_position != right.submit_position) {
      return left.submit_position < right.submit_position;
    }
    return left.position < right.position;
  };
  std::ranges::sort(state.image_accesses, device_order);
  std::ranges::sort(state.buffer_accesses, device_order);

  state.image_uses.reserve(state.image_accesses.size());
  for (const auto &access : state.image_accesses) {
    state.image_uses.push_back({
        .image = access.image,
        .command_buffer = access.command_buffer,
        .position = access.position,
        .submit_position = access.submit_position,
    });
  }
}

} // namespace strobe::rdg::detail
