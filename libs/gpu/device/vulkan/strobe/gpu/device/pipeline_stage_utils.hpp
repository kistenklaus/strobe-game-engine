#pragma once

#include "strobe/gpu/device/pipeline_stage.hpp"

#include <vulkan/vulkan_core.h>

namespace strobe::gpu {

static inline VkPipelineStageFlags2
to_vk_pipeline_stage(PipelineStage flags) {
  VkPipelineStageFlags2 result = 0;

  if ((flags & PipelineStage::indirect_command) != 0) {
    result |= VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
  }

  if ((flags & PipelineStage::index_input) != 0) {
    result |= VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT;
  }

  if ((flags & PipelineStage::vertex_attribute_input) != 0) {
    result |= VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT;
  }

  if ((flags & PipelineStage::vertex_shader) != 0) {
    result |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
  }

  if ((flags & PipelineStage::task_shader) != 0) {
    result |= VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT;
  }

  if ((flags & PipelineStage::mesh_shader) != 0) {
    result |= VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT;
  }

  if ((flags & PipelineStage::fragment_shader) != 0) {
    result |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
  }

  if ((flags & PipelineStage::compute_shader) != 0) {
    result |= VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
  }

  if ((flags & PipelineStage::ray_tracing_shader) != 0) {
    result |= VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR;
  }

  if ((flags & PipelineStage::early_fragment_tests) != 0) {
    result |= VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;
  }

  if ((flags & PipelineStage::late_fragment_tests) != 0) {
    result |= VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
  }

  if ((flags & PipelineStage::color_attachment_output) != 0) {
    result |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
  }

  if ((flags & PipelineStage::transfer) != 0) {
    result |= VK_PIPELINE_STAGE_2_TRANSFER_BIT;
  }

  if ((flags & PipelineStage::host) != 0) {
    result |= VK_PIPELINE_STAGE_2_HOST_BIT;
  }

  if ((flags & PipelineStage::all_graphics) != 0) {
    result |= VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
  }

  if ((flags & PipelineStage::all_commands) != 0) {
    result |= VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
  }

  return result;
}

static inline PipelineStage
from_vk_pipeline_stage(VkPipelineStageFlags2 flags) {
  PipelineStage result = PipelineStage::none;

  if ((flags & VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT) != 0) {
    result |= PipelineStage::indirect_command;
  }

  if ((flags & VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT) != 0) {
    result |= PipelineStage::index_input;
  }

  if ((flags & VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT) != 0) {
    result |= PipelineStage::vertex_attribute_input;
  }

  if ((flags & VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT) != 0) {
    result |= PipelineStage::vertex_shader;
  }

  if ((flags & VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT) != 0) {
    result |= PipelineStage::task_shader;
  }

  if ((flags & VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT) != 0) {
    result |= PipelineStage::mesh_shader;
  }

  if ((flags & VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT) != 0) {
    result |= PipelineStage::fragment_shader;
  }

  if ((flags & VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT) != 0) {
    result |= PipelineStage::compute_shader;
  }

  if ((flags & VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR) != 0) {
    result |= PipelineStage::ray_tracing_shader;
  }

  if ((flags & VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT) != 0) {
    result |= PipelineStage::early_fragment_tests;
  }

  if ((flags & VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT) != 0) {
    result |= PipelineStage::late_fragment_tests;
  }

  if ((flags & VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT) != 0) {
    result |= PipelineStage::color_attachment_output;
  }

  if ((flags & VK_PIPELINE_STAGE_2_TRANSFER_BIT) != 0) {
    result |= PipelineStage::transfer;
  }

  if ((flags & VK_PIPELINE_STAGE_2_HOST_BIT) != 0) {
    result |= PipelineStage::host;
  }

  if ((flags & VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT) != 0) {
    result |= PipelineStage::all_graphics;
  }

  if ((flags & VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT) != 0) {
    result |= PipelineStage::all_commands;
  }

  return result;
}

} // namespace strobe::gpu
