#pragma once

#include "strobe/gpu/device/front_face.hpp"

#include <cassert>
#include <vulkan/vulkan_core.h>

namespace strobe::gpu {

static inline VkFrontFace to_vk_front_face(FrontFace face) noexcept {
  switch (face) {
  case FrontFace::counter_clockwise:
    return VK_FRONT_FACE_COUNTER_CLOCKWISE;

  case FrontFace::clockwise:
    return VK_FRONT_FACE_CLOCKWISE;
  }

  assert(false);
  return VK_FRONT_FACE_COUNTER_CLOCKWISE;
}

static inline FrontFace from_vk_front_face(VkFrontFace face) noexcept {
  switch (face) {
  case VK_FRONT_FACE_COUNTER_CLOCKWISE:
    return FrontFace::counter_clockwise;

  case VK_FRONT_FACE_CLOCKWISE:
    return FrontFace::clockwise;

  default:
    assert(false);
    return FrontFace::counter_clockwise;
  }
}

} // namespace strobe::gpu
