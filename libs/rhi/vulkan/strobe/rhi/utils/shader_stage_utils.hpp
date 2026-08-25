#pragma once

#include "strobe/rhi/types/shader_stage.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

static inline VkShaderStageFlags
to_vk_shader_stage(ShaderStage flags) {
  VkShaderStageFlags result = 0;

  if ((flags & ShaderStage::vertex) != 0) {
    result |= VK_SHADER_STAGE_VERTEX_BIT;
  }

  if ((flags & ShaderStage::tessellation_control) != 0) {
    result |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
  }

  if ((flags & ShaderStage::tessellation_evaluation) != 0) {
    result |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
  }

  if ((flags & ShaderStage::geometry) != 0) {
    result |= VK_SHADER_STAGE_GEOMETRY_BIT;
  }

  if ((flags & ShaderStage::fragment) != 0) {
    result |= VK_SHADER_STAGE_FRAGMENT_BIT;
  }

  if ((flags & ShaderStage::compute) != 0) {
    result |= VK_SHADER_STAGE_COMPUTE_BIT;
  }

  if ((flags & ShaderStage::raygen) != 0) {
    result |= VK_SHADER_STAGE_RAYGEN_BIT_KHR;
  }

  if ((flags & ShaderStage::anyhit) != 0) {
    result |= VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
  }

  if ((flags & ShaderStage::hit) != 0) {
    result |= VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
  }

  if ((flags & ShaderStage::miss) != 0) {
    result |= VK_SHADER_STAGE_MISS_BIT_KHR;
  }

  if ((flags & ShaderStage::intersection) != 0) {
    result |= VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
  }

  if ((flags & ShaderStage::callable) != 0) {
    result |= VK_SHADER_STAGE_CALLABLE_BIT_KHR;
  }

  if ((flags & ShaderStage::task) != 0) {
    result |= VK_SHADER_STAGE_TASK_BIT_EXT;
  }

  if ((flags & ShaderStage::mesh) != 0) {
    result |= VK_SHADER_STAGE_MESH_BIT_EXT;
  }

  return result;
}

static inline ShaderStage
from_vk_shader_stage(VkShaderStageFlags flags) {
  ShaderStage result = ShaderStage::none;

  if ((flags & VK_SHADER_STAGE_VERTEX_BIT) != 0) {
    result |= ShaderStage::vertex;
  }

  if ((flags & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) != 0) {
    result |= ShaderStage::tessellation_control;
  }

  if ((flags & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) != 0) {
    result |= ShaderStage::tessellation_evaluation;
  }

  if ((flags & VK_SHADER_STAGE_GEOMETRY_BIT) != 0) {
    result |= ShaderStage::geometry;
  }

  if ((flags & VK_SHADER_STAGE_FRAGMENT_BIT) != 0) {
    result |= ShaderStage::fragment;
  }

  if ((flags & VK_SHADER_STAGE_COMPUTE_BIT) != 0) {
    result |= ShaderStage::compute;
  }

  if ((flags & VK_SHADER_STAGE_RAYGEN_BIT_KHR) != 0) {
    result |= ShaderStage::raygen;
  }

  if ((flags & VK_SHADER_STAGE_ANY_HIT_BIT_KHR) != 0) {
    result |= ShaderStage::anyhit;
  }

  if ((flags & VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR) != 0) {
    result |= ShaderStage::hit;
  }

  if ((flags & VK_SHADER_STAGE_MISS_BIT_KHR) != 0) {
    result |= ShaderStage::miss;
  }

  if ((flags & VK_SHADER_STAGE_INTERSECTION_BIT_KHR) != 0) {
    result |= ShaderStage::intersection;
  }

  if ((flags & VK_SHADER_STAGE_CALLABLE_BIT_KHR) != 0) {
    result |= ShaderStage::callable;
  }

  if ((flags & VK_SHADER_STAGE_TASK_BIT_EXT) != 0) {
    result |= ShaderStage::task;
  }

  if ((flags & VK_SHADER_STAGE_MESH_BIT_EXT) != 0) {
    result |= ShaderStage::mesh;
  }

  return result;
}

} // namespace strobe::rhi
