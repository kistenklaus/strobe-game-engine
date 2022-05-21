#include "strb/apis/vulkan/Instance.hpp"

#include <cassert>
#include <cstring>

#include "strb/apis/glfw.hpp"
#include "strb/apis/vulkan/assert.hpp"

namespace strb::vulkan {

static void getGlfwInstanceExtentions(strb::vector<strb::string> &rExtentions) {
  uint32_t count;
  const char **ext = glfwGetRequiredInstanceExtensions(&count);
  for (uint32_t i = 0; i < count; i++) {
    bool contained = false;
    for (uint32_t j = 0; j < rExtentions.size(); j++) {
      if (std::strcmp(ext[i], rExtentions[j].c_str()) == 0) {
        contained = true;
        break;
      }
    }
    if (!contained) {
      rExtentions.push_back(strb::string(ext[i]));
    }
  }
}

Instance::Instance(const strb::string &applicationName,
                   const strb::string &engineName,
                   const strb::tuple3i &applicationVersion,
                   const strb::tuple3i &engineVersion,
                   const strb::tuple3i &vulkanApiVersion,
                   const strb::window::WindowApi windowApi,
                   const strb::vector<strb::string> extentions,
                   const strb::vector<strb::string> layers) {
  strb::vector<strb::string> requiredExtentions = extentions;  // copy
  if (windowApi == strb::window::WINDOW_API_GLFW) {
    getGlfwInstanceExtentions(requiredExtentions);
  }
  VkApplicationInfo appInfo;
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pNext = NULL;
  appInfo.pApplicationName = applicationName.c_str();
  appInfo.applicationVersion =
      VK_MAKE_VERSION(applicationVersion.get0(), applicationVersion.get1(),
                      applicationVersion.get2());
  appInfo.pEngineName = engineName.c_str();
  appInfo.engineVersion = VK_MAKE_VERSION(
      engineVersion.get0(), engineVersion.get1(), engineVersion.get2());
  appInfo.apiVersion =
      VK_MAKE_VERSION(vulkanApiVersion.get0(), vulkanApiVersion.get1(),
                      vulkanApiVersion.get2());
  uint32_t n_layersProperties;
  VkResult result;
  result = vkEnumerateInstanceLayerProperties(&n_layersProperties, nullptr);
  ASSERT_VKRESULT(result);
  strb::vector<VkLayerProperties> avaiableLayers(n_layersProperties);
  result = vkEnumerateInstanceLayerProperties(&n_layersProperties,
                                              avaiableLayers.data());
  strb::vector<const char *> supportedLayers;
  for (const strb::string &requestedLayer : layers) {
    bool supported = false;
    for (const VkLayerProperties &properties : avaiableLayers) {
      if (std::strcmp(properties.layerName, requestedLayer.c_str()) == 0) {
        supportedLayers.push_back(properties.layerName);
        supported = true;
        break;
      }
    }
    if (!supported) {
      std::cerr << "WARNING: Layer [" << requestedLayer << "] is not supported."
                << std::endl;
    }
  }
  uint32_t n_extentionProperties;
  result = vkEnumerateInstanceExtensionProperties(
      nullptr, &n_extentionProperties, nullptr);
  ASSERT_VKRESULT(result);
  strb::vector<VkExtensionProperties> avaiableExtentions(n_extentionProperties);
  result = vkEnumerateInstanceExtensionProperties(
      nullptr, &n_extentionProperties, avaiableExtentions.data());
  ASSERT_VKRESULT(result);
  strb::vector<const char *> supportedExtentions;
  for (const strb::string &requestedExtention : requiredExtentions) {
    strb::boolean supported = false;
    for (const VkExtensionProperties &properties : avaiableExtentions) {
      if (std::strcmp(properties.extensionName, requestedExtention.c_str()) ==
          0) {
        supportedExtentions.push_back(properties.extensionName);
        supported = true;
        break;
      }
    }
    if (!supported) {
      std::cerr << "WARNING: Extension [" << requestedExtention
                << "] is not supported." << std::endl;
    }
  }
  VkInstanceCreateInfo instanceCreateInfo;
  instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instanceCreateInfo.pNext = nullptr;
  instanceCreateInfo.flags = 0;
  instanceCreateInfo.pApplicationInfo = &appInfo;
  instanceCreateInfo.enabledLayerCount = supportedLayers.size();
  instanceCreateInfo.ppEnabledLayerNames = supportedLayers.data();
  instanceCreateInfo.enabledExtensionCount = supportedExtentions.size();
  instanceCreateInfo.ppEnabledExtensionNames = supportedExtentions.data();
  result = vkCreateInstance(&instanceCreateInfo, nullptr, &this->instance);
  ASSERT_VKRESULT(result);
  assert(this->instance != VK_NULL_HANDLE);
}

void Instance::destroy() {
  assert(this->instance != VK_NULL_HANDLE);
  vkDestroyInstance(this->instance, nullptr);
  dexec(this->instance = VK_NULL_HANDLE);
}

}  // namespace strb::vulkan
