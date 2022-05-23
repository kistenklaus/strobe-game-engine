#pragma once
#include "strb/apis/vulkan/Device.hpp"
#include "strb/apis/vulkan/RenderPass.hpp"
#include "strb/apis/vulkan/Shader.hpp"
#include "strb/apis/vulkan/VertexInputLayout.hpp"
#include "strb/apis/vulkan/vulkan.lib.hpp"

namespace strb::vulkan {

struct Pipeline {
private:
  const Device *device;
  const RenderPass *renderPass;
  const VertexLayout *inputLayout;
  VkPipelineLayout pipelineLayout;
  VkPipeline pipeline;
  uint32_t width;
  uint32_t height;
  strb::optional<Shader> vertexShader;
  strb::optional<Shader> fragmentShader;

public:
  inline Pipeline()
      : device(nullptr), renderPass(nullptr), pipelineLayout(VK_NULL_HANDLE),
        pipeline(VK_NULL_HANDLE) {}
  Pipeline(const Device &device, const RenderPass &renderPass,
           const VertexLayout &inputLayout, const uint32_t width,
           const uint32_t height, strb::optional<Shader> vertexShader,
           strb::optional<Shader> fragmentShader);
  void destroy();
  void bind(const CommandBuffer &commandBuffer);
  void executeDrawCall(const CommandBuffer &commandBuffer,
                       const uint32_t vertexCount, const uint32_t instanceCount,
                       const uint32_t vertexOffset,
                       const uint32_t instanceOffset);
  void unbind();
  inline operator VkPipeline() const { return pipeline; }
  void recreate(const uint32_t width, const uint32_t height);
};

} // namespace strb::vulkan
