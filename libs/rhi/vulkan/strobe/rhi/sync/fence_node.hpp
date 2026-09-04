#pragma once

#include "strobe/rhi/vulkan/fence.hpp"
namespace strobe::rhi {

struct FenceNode {
  vulkan::Fence fence;
  std::mutex mutex;
  FenceNode* next;
};

}
