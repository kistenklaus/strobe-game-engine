#include "compiler.hpp"
#include "util.hpp"

#include <algorithm>

namespace strobe::rdg::detail {

void derive_device_order(flow::detail::compiler::NormalizedIR &,
                         flow::CompileContext &context) {
  auto &state = compiler_state(context);

  for (auto &marker : state.image_markers) {
    marker.submit_position = submission_for(
        state.submissions, marker.command_buffer, marker.position);
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
  std::ranges::sort(state.image_markers, device_order);
  std::ranges::sort(state.buffer_accesses, device_order);

  state.image_uses.reserve(state.image_markers.size());
  for (const auto &marker : state.image_markers) {
    if (marker.kind != ImageMarkerKind::access) {
      continue;
    }
    state.image_uses.push_back({
        .image = marker.image,
        .command_buffer = marker.command_buffer,
        .position = marker.position,
        .submit_position = marker.submit_position,
    });
  }
}

} // namespace strobe::rdg::detail
