#pragma once

#include "strobe/rhi/types/stencil_face.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

static inline VkStencilFaceFlags
to_vk_stencil_face(StencilFace face) noexcept {
  VkStencilFaceFlags result = 0;

  if ((face & StencilFace::front) != 0) {
    result |= VK_STENCIL_FACE_FRONT_BIT;
  }

  if ((face & StencilFace::back) != 0) {
    result |= VK_STENCIL_FACE_BACK_BIT;
  }

  return result;
}

static inline StencilFace
from_vk_stencil_face(VkStencilFaceFlags face) noexcept {
  StencilFace result = StencilFace::none;

  if ((face & VK_STENCIL_FACE_FRONT_BIT) != 0) {
    result |= StencilFace::front;
  }

  if ((face & VK_STENCIL_FACE_BACK_BIT) != 0) {
    result |= StencilFace::back;
  }

  return result;
}

} // namespace strobe::rhi
