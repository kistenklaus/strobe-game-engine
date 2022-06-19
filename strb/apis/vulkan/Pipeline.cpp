#include "strb/apis/vulkan/Pipeline.hpp"

#include <cassert>

#include "strb/apis/vulkan/assert.hpp"

namespace strb::vulkan {

Pipeline::Pipeline(const Device &device, const RenderPass &renderPass,
                   const uint32_t width, const uint32_t height,
                   strb::optional<Shader> vertexShader,
                   strb::optional<Shader> fragmentShader)
    : device(&device),
      renderPass(&renderPass),
      vertexShader(vertexShader),
      fragmentShader(fragmentShader) {
  // Create Pipeline layout
  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
  pipelineLayoutCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutCreateInfo.pNext = nullptr;
  pipelineLayoutCreateInfo.flags = 0;
  pipelineLayoutCreateInfo.setLayoutCount = 0;
  pipelineLayoutCreateInfo.pSetLayouts = nullptr;
  pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
  pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

  VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo,
                                           nullptr, &this->pipelineLayout);
  ASSERT_VKRESULT(result);

  // Create Pipeline.
  VkViewport viewport;
  viewport.x = 0.0;
  viewport.y = 0.0;
  viewport.width = width;
  viewport.height = height;
  viewport.minDepth = 0.0;
  viewport.maxDepth = 1.0;
  VkRect2D scissor;
  scissor.offset = {0, 0};
  scissor.extent = {width, height};

  strb::optional<VkShaderModule> vertexShaderModule;
  if (vertexShader.has_value()) {
    vertexShaderModule = vertexShader.value();
  }
  strb::optional<VkShaderModule> fragmentShaderModule;
  if (fragmentShader.has_value()) {
    fragmentShaderModule = fragmentShader.value();
  }

  VkPipelineShaderStageCreateInfo shaderStageCreateInfoVert;
  shaderStageCreateInfoVert.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStageCreateInfoVert.pNext = nullptr;
  shaderStageCreateInfoVert.flags = 0;
  shaderStageCreateInfoVert.stage = VK_SHADER_STAGE_VERTEX_BIT;
  shaderStageCreateInfoVert.module = vertexShader.value();
  shaderStageCreateInfoVert.pName = "main";
  shaderStageCreateInfoVert.pSpecializationInfo = nullptr;

  VkPipelineShaderStageCreateInfo shaderStageCreateInfoFrag;
  shaderStageCreateInfoFrag.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStageCreateInfoFrag.pNext = nullptr;
  shaderStageCreateInfoFrag.flags = 0;
  shaderStageCreateInfoFrag.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  shaderStageCreateInfoFrag.module = fragmentShader.value();
  shaderStageCreateInfoFrag.pName = "main";
  shaderStageCreateInfoFrag.pSpecializationInfo = nullptr;

  VkPipelineShaderStageCreateInfo shaderStages[] = {shaderStageCreateInfoVert,
                                                    shaderStageCreateInfoFrag};
  VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo;
  vertexInputCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputCreateInfo.pNext = nullptr;
  vertexInputCreateInfo.flags = 0;
  vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
  vertexInputCreateInfo.pVertexBindingDescriptions = nullptr;
  vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
  vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;

  VkPipelineInputAssemblyStateCreateInfo inputAssemlyCreateInfo;
  inputAssemlyCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssemlyCreateInfo.pNext = nullptr;
  inputAssemlyCreateInfo.flags = 0;
  inputAssemlyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssemlyCreateInfo.primitiveRestartEnable = VK_FALSE;

  VkPipelineViewportStateCreateInfo viewportStateCreateInfo;
  viewportStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportStateCreateInfo.pNext = nullptr;
  viewportStateCreateInfo.flags = 0;
  viewportStateCreateInfo.viewportCount = 1;
  viewportStateCreateInfo.pViewports = &viewport;
  viewportStateCreateInfo.scissorCount = 1;
  viewportStateCreateInfo.pScissors = &scissor;

  VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo;
  rasterizationStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizationStateCreateInfo.pNext = nullptr;
  rasterizationStateCreateInfo.flags = 0;
  rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
  rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
  rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
  rasterizationStateCreateInfo.depthBiasClamp = 0.0;
  rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
  rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;
  rasterizationStateCreateInfo.lineWidth = 1.0f;

  VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo;
  multisampleStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampleStateCreateInfo.pNext = nullptr;
  multisampleStateCreateInfo.flags = 0;
  multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
  multisampleStateCreateInfo.minSampleShading = 1.0;
  multisampleStateCreateInfo.pSampleMask = nullptr;
  multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
  multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

  VkPipelineColorBlendAttachmentState colorBlendAttachmentState;
  colorBlendAttachmentState.blendEnable = VK_TRUE;
  colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  colorBlendAttachmentState.dstColorBlendFactor =
      VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
  colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
  colorBlendAttachmentState.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

  VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo;
  colorBlendCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlendCreateInfo.pNext = nullptr;
  colorBlendCreateInfo.flags = 0;
  colorBlendCreateInfo.logicOpEnable = VK_FALSE;
  colorBlendCreateInfo.logicOp = VK_LOGIC_OP_NO_OP;  // not used then VK_FALSE
  colorBlendCreateInfo.attachmentCount = 1;
  colorBlendCreateInfo.pAttachments = &colorBlendAttachmentState;
  colorBlendCreateInfo.blendConstants[0] = 0.0;
  colorBlendCreateInfo.blendConstants[1] = 0.0;
  colorBlendCreateInfo.blendConstants[2] = 0.0;
  colorBlendCreateInfo.blendConstants[3] = 0.0;
  VkGraphicsPipelineCreateInfo pipelineCreateInfo;
  pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineCreateInfo.pNext = nullptr;
  pipelineCreateInfo.flags = 0;
  pipelineCreateInfo.stageCount = 2;
  pipelineCreateInfo.pStages = shaderStages;
  pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
  pipelineCreateInfo.pInputAssemblyState = &inputAssemlyCreateInfo;
  pipelineCreateInfo.pTessellationState = nullptr;
  pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
  pipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
  pipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
  pipelineCreateInfo.pDepthStencilState = nullptr;
  pipelineCreateInfo.pColorBlendState = &colorBlendCreateInfo;
  pipelineCreateInfo.pDynamicState = nullptr;
  pipelineCreateInfo.layout = pipelineLayout;
  pipelineCreateInfo.renderPass = renderPass;
  pipelineCreateInfo.subpass = 0;
  pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineCreateInfo.basePipelineIndex = -1;

  result = vkCreateGraphicsPipelines(
      device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &this->pipeline);
  ASSERT_VKRESULT(result);
}

void Pipeline::destroy() {
  assert(device != nullptr);
  assert(pipelineLayout != VK_NULL_HANDLE);
  assert(pipeline != VK_NULL_HANDLE);
  vkDestroyPipeline(*device, this->pipeline, nullptr);
  vkDestroyPipelineLayout(*device, this->pipelineLayout, nullptr);
  dexec(this->device = nullptr);
  dexec(this->pipeline = VK_NULL_HANDLE);
  dexec(this->pipelineLayout = VK_NULL_HANDLE);
}

void Pipeline::bind(const CommandBuffer &commandBuffer) {
  assert(device != nullptr);
  assert(pipelineLayout != VK_NULL_HANDLE);
  assert(pipeline != VK_NULL_HANDLE);

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    this->pipeline);
}

void Pipeline::executeDrawCall(const CommandBuffer &commandBuffer,
                               const uint32_t vertexCount,
                               const uint32_t instanceCount,
                               const uint32_t vertexOffset,
                               const uint32_t instanceOffset) {
  assert(device != nullptr);
  assert(pipelineLayout != VK_NULL_HANDLE);
  assert(pipeline != VK_NULL_HANDLE);
  vkCmdDraw(commandBuffer, vertexCount, instanceCount, vertexOffset,
            instanceOffset);
}

void Pipeline::unbind() { assert(device != nullptr); }

void Pipeline::recreate(const uint32_t width, const uint32_t height) {
  vkDestroyPipeline(*this->device, this->pipeline, nullptr);
  vkDestroyPipelineLayout(*this->device, this->pipelineLayout, nullptr);

  this->width = width;
  this->height = height;
  // Create Pipeline layout
  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
  pipelineLayoutCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutCreateInfo.pNext = nullptr;
  pipelineLayoutCreateInfo.flags = 0;
  pipelineLayoutCreateInfo.setLayoutCount = 0;
  pipelineLayoutCreateInfo.pSetLayouts = nullptr;
  pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
  pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

  VkResult result = vkCreatePipelineLayout(
      *this->device, &pipelineLayoutCreateInfo, nullptr, &this->pipelineLayout);
  ASSERT_VKRESULT(result);

  // Create Pipeline.
  VkViewport viewport;
  viewport.x = 0.0;
  viewport.y = 0.0;
  viewport.width = width;
  viewport.height = height;
  viewport.minDepth = 0.0;
  viewport.maxDepth = 1.0;
  VkRect2D scissor;
  scissor.offset = {0, 0};
  scissor.extent = {width, height};

  strb::optional<VkShaderModule> vertexShaderModule;
  if (vertexShader.has_value()) {
    vertexShaderModule = vertexShader.value();
  }
  strb::optional<VkShaderModule> fragmentShaderModule;
  if (fragmentShader.has_value()) {
    fragmentShaderModule = fragmentShader.value();
  }

  VkPipelineShaderStageCreateInfo shaderStageCreateInfoVert;
  shaderStageCreateInfoVert.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStageCreateInfoVert.pNext = nullptr;
  shaderStageCreateInfoVert.flags = 0;
  shaderStageCreateInfoVert.stage = VK_SHADER_STAGE_VERTEX_BIT;
  shaderStageCreateInfoVert.module = vertexShader.value();
  shaderStageCreateInfoVert.pName = "main";
  shaderStageCreateInfoVert.pSpecializationInfo = nullptr;

  VkPipelineShaderStageCreateInfo shaderStageCreateInfoFrag;
  shaderStageCreateInfoFrag.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStageCreateInfoFrag.pNext = nullptr;
  shaderStageCreateInfoFrag.flags = 0;
  shaderStageCreateInfoFrag.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  shaderStageCreateInfoFrag.module = fragmentShader.value();
  shaderStageCreateInfoFrag.pName = "main";
  shaderStageCreateInfoFrag.pSpecializationInfo = nullptr;

  VkPipelineShaderStageCreateInfo shaderStages[] = {shaderStageCreateInfoVert,
                                                    shaderStageCreateInfoFrag};
  VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo;
  vertexInputCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputCreateInfo.pNext = nullptr;
  vertexInputCreateInfo.flags = 0;
  vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
  vertexInputCreateInfo.pVertexBindingDescriptions = nullptr;
  vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
  vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;

  VkPipelineInputAssemblyStateCreateInfo inputAssemlyCreateInfo;
  inputAssemlyCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssemlyCreateInfo.pNext = nullptr;
  inputAssemlyCreateInfo.flags = 0;
  inputAssemlyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssemlyCreateInfo.primitiveRestartEnable = VK_FALSE;

  VkPipelineViewportStateCreateInfo viewportStateCreateInfo;
  viewportStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportStateCreateInfo.pNext = nullptr;
  viewportStateCreateInfo.flags = 0;
  viewportStateCreateInfo.viewportCount = 1;
  viewportStateCreateInfo.pViewports = &viewport;
  viewportStateCreateInfo.scissorCount = 1;
  viewportStateCreateInfo.pScissors = &scissor;

  VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo;
  rasterizationStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizationStateCreateInfo.pNext = nullptr;
  rasterizationStateCreateInfo.flags = 0;
  rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
  rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
  rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
  rasterizationStateCreateInfo.depthBiasClamp = 0.0;
  rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
  rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;
  rasterizationStateCreateInfo.lineWidth = 1.0f;

  VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo;
  multisampleStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampleStateCreateInfo.pNext = nullptr;
  multisampleStateCreateInfo.flags = 0;
  multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
  multisampleStateCreateInfo.minSampleShading = 1.0;
  multisampleStateCreateInfo.pSampleMask = nullptr;
  multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
  multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

  VkPipelineColorBlendAttachmentState colorBlendAttachmentState;
  colorBlendAttachmentState.blendEnable = VK_TRUE;
  colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  colorBlendAttachmentState.dstColorBlendFactor =
      VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
  colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
  colorBlendAttachmentState.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

  VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo;
  colorBlendCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlendCreateInfo.pNext = nullptr;
  colorBlendCreateInfo.flags = 0;
  colorBlendCreateInfo.logicOpEnable = VK_FALSE;
  colorBlendCreateInfo.logicOp = VK_LOGIC_OP_NO_OP;  // not used then VK_FALSE
  colorBlendCreateInfo.attachmentCount = 1;
  colorBlendCreateInfo.pAttachments = &colorBlendAttachmentState;
  colorBlendCreateInfo.blendConstants[0] = 0.0;
  colorBlendCreateInfo.blendConstants[1] = 0.0;
  colorBlendCreateInfo.blendConstants[2] = 0.0;
  colorBlendCreateInfo.blendConstants[3] = 0.0;

  VkGraphicsPipelineCreateInfo pipelineCreateInfo;
  pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineCreateInfo.pNext = nullptr;
  pipelineCreateInfo.flags = 0;
  pipelineCreateInfo.stageCount = 2;
  pipelineCreateInfo.pStages = shaderStages;
  pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
  pipelineCreateInfo.pInputAssemblyState = &inputAssemlyCreateInfo;
  pipelineCreateInfo.pTessellationState = nullptr;
  pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
  pipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
  pipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
  pipelineCreateInfo.pDepthStencilState = nullptr;
  pipelineCreateInfo.pColorBlendState = &colorBlendCreateInfo;
  pipelineCreateInfo.pDynamicState = nullptr;
  pipelineCreateInfo.layout = pipelineLayout;
  pipelineCreateInfo.renderPass = *this->renderPass;
  pipelineCreateInfo.subpass = 0;
  pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineCreateInfo.basePipelineIndex = -1;

  result =
      vkCreateGraphicsPipelines(*this->device, VK_NULL_HANDLE, 1,
                                &pipelineCreateInfo, nullptr, &this->pipeline);
  ASSERT_VKRESULT(result);
}

}  // namespace strb::vulkan
