#pragma once

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Device feature configuration.
 *
 * Selects optional device features and extensions enabled during device
 * creation.
 */
struct DeviceInfo {
  /** Enable debug utility support. */
  bool debug_utils = false;

  /** Enable swapchain and presentation support. */
  bool swapchain = true;

  /** Enable shader object support. */
  bool shaders = true;

  /** Enable ray tracing pipeline and acceleration structure support. */
  bool raytracing = true;

  /** Enable ray query support. */
  bool rayQuery = true;
};

} // namespace strobe::rhi
