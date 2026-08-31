#pragma once

namespace strobe::rhi {

struct DeviceInfo {
  bool debug_utils = false;
  bool swapchain = true;
  bool shaders = true;
  bool raytracing = true;
  bool rayQuery = true;
};

}
