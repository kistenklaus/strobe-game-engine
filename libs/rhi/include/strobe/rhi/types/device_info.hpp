#pragma once

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief todo
 */
struct DeviceInfo {
  bool debug_utils = false;
  bool swapchain = true;
  bool shaders = true;
  bool raytracing = true;
  bool rayQuery = true;
};

}
