#pragma once

#include <expected>
#include <vulkan/vulkan.hpp>

namespace strobe::renderer::vks { 

class VksContext {
  public:
    std::expected<VksContext, vk::Result> createInstance(
        std::string_view appName,) {
      vk::ApplicationInfo appInfo;     
      /* appInfo.pApplicationName =  */
    }

  private:
    VksContext(vk::Instance instance, vk::Device device, 
        vk::PhysicalDevice physicalDevice) {

    }
  
  public:
    vk::Instance instance;
    vk::Device device;
    vk::PhysicalDevice physicalDevice;
};


}
