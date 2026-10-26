#include "strobe/rdg/plugins.hpp"

#include "detail/compiler/compiler.hpp"

namespace strobe::rdg {

void append_plugins(flow::CompilerPlugins &plugins) {
  plugins.normalized.emplace_back(detail::define_globals);
  // barrier-placement:
  plugins.normalized.emplace_back(detail::collect_submissions);
  plugins.normalized.emplace_back(detail::collect_resource_states);
  plugins.normalized.emplace_back(detail::collect_resource_accesses);
  plugins.normalized.emplace_back(detail::derive_device_order);
  plugins.normalized.emplace_back(detail::place_image_barriers);
  plugins.normalized.emplace_back(detail::place_buffer_barriers);
  plugins.normalized.emplace_back(detail::remove_redundant_accesses);

  plugins.normalized.emplace_back(detail::materialize_operations);

  plugins.normalized.emplace_back(detail::lower_swapchain);
}

} // namespace strobe::rdg
