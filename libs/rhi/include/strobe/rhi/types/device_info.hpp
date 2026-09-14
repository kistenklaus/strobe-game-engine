#pragma once

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Device feature configuration.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} device_info.hpp DeviceInfo
 *
 * Selects optional device features and extensions enabled during device
 * creation.
 */
// [DeviceInfo]
struct DeviceInfo {
  bool debug_utils = false;
  bool swapchain = true;
  bool shaders = true;
  bool raytracing = true;
  bool rayQuery = true;
};
// [DeviceInfo]

} // namespace strobe::rhi
