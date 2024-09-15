#pragma once

#include <vulkan/vulkan_core.h>
#include <span>
#include <stdexcept>
#include "./vk.h"

namespace strobe::vk {

struct ShaderModule {
  public:
    ShaderModule() : m_handle(VK_NULL_HANDLE) {}
    static ShaderModule create(const std::span<char>& code);
    ~ShaderModule();

   private:
    explicit ShaderModule(VkShaderModule handle) : m_handle(handle) {}
    VkShaderModule m_handle;
};

}
