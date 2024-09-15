#include "./vk.h"

#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <ranges>
#include <stdexcept>
#include <vector>

#include "../glfw/glfw.h"
#include "./ext.h"

constexpr std::array<const char*, 1> REQUIRED_LAYERS = {
    "VK_LAYER_KHRONOS_validation"};
constexpr std::array<const char*, 1> REQUIRED_DEVICE_EXTENSIONS = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

VkInstance strobe::vk::g_instance;
VkDebugUtilsMessengerEXT strobe::vk::g_debugger;
VkDevice strobe::vk::g_device;
VkPhysicalDevice strobe::vk::g_physicalDevice;
uint32_t strobe::vk::g_gfxQueueFamilyIndex;
VkQueue strobe::vk::g_gfxQueue;
uint32_t strobe::vk::g_presentQueueFamilyIndex;
VkQueue strobe::vk::g_presentQueue;
VkSurfaceKHR strobe::vk::g_surface;
VkSwapchainKHR strobe::vk::g_swapchain;
VkFormat strobe::vk::g_swapchainFormat;
std::vector<VkImage> strobe::vk::g_swapchainImages;
std::vector<VkImageView> strobe::vk::g_swapchainImageViews;

using namespace strobe::vk;

static void logSupportedInstanceLayers();
static void logSupportedInstanceExtensions();
static void initInstance();
static void initSurface();
static std::vector<VkPhysicalDevice> getPhysicalDevices();
static void logPhysicalDevices(const std::vector<VkPhysicalDevice>&);
static VkPhysicalDevice selectPhysicalDevice(
    const std::vector<VkPhysicalDevice>&);
static std::vector<VkQueueFamilyProperties> getQueueFamilies(VkPhysicalDevice);
static void logQueueFamilies(const std::vector<VkQueueFamilyProperties>&);
static void initPhysicalDevice();
static uint32_t selectGraphicsQueue(
    const std::vector<VkQueueFamilyProperties>&);
static uint32_t selectPresentQueue(VkPhysicalDevice,
                                   const std::vector<VkQueueFamilyProperties>&);
static std::vector<VkExtensionProperties> getDeviceExtensions(VkPhysicalDevice);
static bool checkDeviceExtensionSupport(
    const std::vector<VkExtensionProperties>&);
static void initDeviceAndQueues();
static VkSurfaceCapabilitiesKHR getSurfaceCapabilities(VkPhysicalDevice);
static std::vector<VkSurfaceFormatKHR> getSurfaceFormats(VkPhysicalDevice);
static std::vector<VkPresentModeKHR> getSurfacePresentModes(VkPhysicalDevice);
static VkSurfaceFormatKHR selectSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>&);
static VkPresentModeKHR selectPresentMode(const std::vector<VkPresentModeKHR>&);
static VkExtent2D selectSwapExtend(const VkSurfaceCapabilitiesKHR& caps);
static void initSwapchain();
static std::vector<VkImage> getSwapchainImages();
static void initSwapchainImageViews() {
  auto images = getSwapchainImages();
  auto imageCount = images.size();
  g_swapchainImageViews.resize(imageCount);

  for (const auto& [i, image] : std::views::enumerate(images)) {
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = image;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = g_swapchainFormat;
    createInfo.components = {
        .r = VK_COMPONENT_SWIZZLE_IDENTITY,
        .g = VK_COMPONENT_SWIZZLE_IDENTITY,
        .b = VK_COMPONENT_SWIZZLE_IDENTITY,
        .a = VK_COMPONENT_SWIZZLE_IDENTITY,
    };
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(g_device, &createInfo, nullptr,
                          &g_swapchainImageViews[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create image views!");
    }
  }
}

void strobe::vk::init() {
  /* logSupportedInstanceLayers(); */
  /* logSupportedInstanceExtensions(); */
  initInstance();
  initSurface();
  initPhysicalDevice();
  initDeviceAndQueues();
  initSwapchain();
}
void strobe::vk::destroy() {
  for (auto imageView : g_swapchainImageViews) {
    vkDestroyImageView(g_device, imageView, nullptr);
  }
  vkDestroySwapchainKHR(g_device, g_swapchain, nullptr);

  vkDestroyDevice(g_device, nullptr);

  vkDestroySurfaceKHR(g_instance, g_surface, nullptr);

  ext::vkDestroyDebugUtilsMessengerEXT(g_instance, g_debugger, nullptr);

  vkDestroyInstance(g_instance, nullptr);
}

/*
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
              void* pUserData) {
  std::cout << pCallbackData->pMessage << std::endl;

  return VK_FALSE;
}

static void logSupportedInstanceExtensions() {
  uint32_t count;
  vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
  std::vector<VkExtensionProperties> supportedExtensions(count);
  vkEnumerateInstanceExtensionProperties(nullptr, &count,
                                         supportedExtensions.data());
  std::cout << "==============SUPPORTED-EXTENSIONS==============" << std::endl;
  for (const auto& extension : supportedExtensions) {
    std::cout << "-" << extension.extensionName << std::endl;
    std::cout << "    SPEC-VERSION: " << VK_VERSION_MAJOR(extension.specVersion)
              << "." << VK_VERSION_MINOR(extension.specVersion) << "."
              << VK_VERSION_PATCH(extension.specVersion) << std::endl;
  }
}
static void logSupportedInstanceLayers() {
  uint32_t count = 0;
  vkEnumerateInstanceLayerProperties(&count, nullptr);
  std::vector<VkLayerProperties> supportedLayers(count);
  vkEnumerateInstanceLayerProperties(&count, supportedLayers.data());
  std::cout << "==================SUPPORTED-LAYERS==============" << std::endl;
  for (const auto& prop : supportedLayers) {
    std::cout << "-" << prop.layerName << std::endl;
    std::cout << "    SPEC-VERSION: " << VK_VERSION_MAJOR(prop.specVersion)
              << "." << VK_VERSION_MINOR(prop.specVersion) << "."
              << VK_VERSION_PATCH(prop.specVersion) << std::endl;
    std::cout << "    IMPL-VERSION: "
              << VK_VERSION_MAJOR(prop.implementationVersion) << "."
              << VK_VERSION_MINOR(prop.implementationVersion) << "."
              << VK_VERSION_PATCH(prop.implementationVersion) << std::endl;
    std::cout << "     DESCRIPTION: " << prop.description << std::endl;
  }
  std::cout << "================================================" << std::endl;
}

static void initInstance() {
  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pNext = nullptr;
  appInfo.pApplicationName = "testing";
  appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
  appInfo.pEngineName = "strobe";
  appInfo.engineVersion = VK_MAKE_VERSION(0, 10, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo instanceCreateInfo{};
  instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instanceCreateInfo.pNext = nullptr;
  instanceCreateInfo.flags = 0;
  instanceCreateInfo.pApplicationInfo = &appInfo;
  instanceCreateInfo.enabledLayerCount = REQUIRED_LAYERS.size();
  instanceCreateInfo.ppEnabledLayerNames = REQUIRED_LAYERS.data();

  std::vector<const char*> extensions;
  {
    uint32_t count;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&count);
    extensions = std::vector(glfwExtensions, glfwExtensions + count);
  }
  extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  instanceCreateInfo.enabledExtensionCount = extensions.size();
  instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

  VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo{};
  debugUtilsMessengerCreateInfo.sType =
      VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  debugUtilsMessengerCreateInfo.messageSeverity =
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  debugUtilsMessengerCreateInfo.messageType =
      VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  debugUtilsMessengerCreateInfo.pfnUserCallback = debugCallback;
  debugUtilsMessengerCreateInfo.pUserData = nullptr;  // Optional

  instanceCreateInfo.pNext = &debugUtilsMessengerCreateInfo;

  {
    VkResult r = vkCreateInstance(&instanceCreateInfo, nullptr, &g_instance);
    if (r != VK_SUCCESS) {
      throw std::runtime_error("Failed to create vulkan instance");
    }
  }
  {
    VkResult r = strobe::vk::ext::vkCreateDebugUtilsMessengerEXT(
        g_instance, &debugUtilsMessengerCreateInfo, nullptr, &g_debugger);
    if (r != VK_SUCCESS) {
      throw std::runtime_error("Failed to create vulkan debugger");
    }
  }
}

static void initSurface() {
  VkResult r = glfwCreateWindowSurface(g_instance, strobe::glfw::g_window,
                                       nullptr, &g_surface);
  if (r != VK_SUCCESS) {
    throw std::runtime_error("Failed to create Window Surface");
  }
}

static void logPhysicalDevices(const std::vector<VkPhysicalDevice>& v) {
  for (const auto& d : v) {
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(d, &props);
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(d, &features);
    std::cout << "-" << props.deviceName << std::endl;
  }
}

static std::vector<VkPhysicalDevice> getPhysicalDevices() {
  uint32_t count;
  {
    VkResult r = vkEnumeratePhysicalDevices(g_instance, &count, nullptr);
    if (r != VK_SUCCESS) {
      throw std::runtime_error("Failed to Enumerate physical devices");
    }
  }
  std::vector<VkPhysicalDevice> physicalDevices(count);
  {
    VkResult r =
        vkEnumeratePhysicalDevices(g_instance, &count, physicalDevices.data());
    if (r != VK_SUCCESS) {
      throw std::runtime_error("Failed to Enumerate physical devices");
    }
  }
  return physicalDevices;
}

static VkPhysicalDevice selectPhysicalDevice(
    const std::vector<VkPhysicalDevice>& physicalDevices) {
  int maxScore = 0;
  bool notFound = true;
  VkPhysicalDevice physicalDevice;
  for (const auto& pd : physicalDevices) {
    // filtering
    bool extensionSupported =
        checkDeviceExtensionSupport(getDeviceExtensions(pd));
    if (extensionSupported) {
      continue;
    }

    auto formats = getSurfaceFormats(pd);
    if (formats.empty()) {
      continue;
    }

    auto presentModes = getSurfacePresentModes(pd);
    if (presentModes.empty()) {
      continue;
    }

    // scoring
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(pd, &properties);
    int score = 0;
    if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      score += 1000;
    }
    score += properties.limits.maxImageDimension2D;
    if (score > maxScore) {
      notFound = false;
      maxScore = score;
      physicalDevice = pd;
    }
  }
  return physicalDevice;
}

static std::vector<VkQueueFamilyProperties> getQueueFamilies(
    VkPhysicalDevice physicalDevice) {
  uint32_t count;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);
  std::vector<VkQueueFamilyProperties> queueFamilies;
  queueFamilies.resize(count);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count,
                                           queueFamilies.data());
  return queueFamilies;
}

static void logQueueFamilies(
    const std::vector<VkQueueFamilyProperties>& queueFamilies) {
  std::cout << "=================QUEUE-FAMILIES================" << std::endl;
  for (const auto& [i, props] : std::views::enumerate(queueFamilies)) {
    std::cout << "-Queue: " << i << std::endl;
    std::cout << "          GRAPHICS-BIT: "
              << ((props.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) << std::endl;
    std::cout << "           COMPUTE_BIT: "
              << ((props.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) << std::endl;
    std::cout << "          TRANSFER-BIT: "
              << ((props.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) << std::endl;
    std::cout << "    SPARSE-BINDING-BIT: "
              << ((props.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) << std::endl;
    std::cout << "         PROTECTED-BIT: "
              << ((props.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) << std::endl;
    std::cout << "  VIDEO-DECODE-BIT-KHR: "
              << ((props.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) << std::endl;
    std::cout << "  VIDEO-ENCODE-BIT-KHR: "
              << ((props.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) << std::endl;
    std::cout << "   OPTICAL-FLOW-BIT-NV: "
              << ((props.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) << std::endl;
    std::cout << "   COUNT: " << props.queueCount << std::endl;
  }
  std::cout << "================================================" << std::endl;
}

static uint32_t selectGraphicsQueue(
    const std::vector<VkQueueFamilyProperties>& queueFamilies) {
  std::optional<uint32_t> gfxQueueFamilyIndex;
  for (const auto& [i, queueFamily] :
       std::ranges::enumerate_view(queueFamilies)) {
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      gfxQueueFamilyIndex = i;
    }
  }
  if (!gfxQueueFamilyIndex) {
    throw std::runtime_error(
        "GPU does not support a queue with graphcis support.");
  }
  return *gfxQueueFamilyIndex;
}

static uint32_t selectPresentQueue(
    VkPhysicalDevice physicalDevice,
    const std::vector<VkQueueFamilyProperties>& queueFamilies) {
  std::optional<uint32_t> presentQueueFamilyIndex;
  for (const auto& [i, queueFamily] :
       std::ranges::enumerate_view(queueFamilies)) {
    VkBool32 presentSupport = false;
    VkResult r = vkGetPhysicalDeviceSurfaceSupportKHR(
        physicalDevice, i, g_surface, &presentSupport);
    if (r != VK_SUCCESS) {
      throw std::runtime_error(
          "Failed to fetch physical device surface support");
    }
    if (presentSupport) {
      presentQueueFamilyIndex = i;
    }
  }
  assert(presentQueueFamilyIndex);
  return *presentQueueFamilyIndex;
}

static void initDeviceAndQueues() {
  auto queueFamilies = getQueueFamilies(g_physicalDevice);
  g_gfxQueueFamilyIndex = selectGraphicsQueue(queueFamilies);
  g_presentQueueFamilyIndex =
      selectPresentQueue(g_physicalDevice, queueFamilies);

  VkDeviceQueueCreateInfo deviceGfxQueueCreateInfo{};
  deviceGfxQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  deviceGfxQueueCreateInfo.pNext = nullptr;
  deviceGfxQueueCreateInfo.flags = 0;
  deviceGfxQueueCreateInfo.queueFamilyIndex = g_gfxQueueFamilyIndex;
  std::array<float, 1> gfxQueuePriorities = {1.0f};
  deviceGfxQueueCreateInfo.queueCount = gfxQueuePriorities.size();
  deviceGfxQueueCreateInfo.pQueuePriorities = gfxQueuePriorities.data();

  VkDeviceQueueCreateInfo devicePresentQueueCreateInfo{};
  devicePresentQueueCreateInfo.sType =
      VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  devicePresentQueueCreateInfo.pNext = nullptr;
  devicePresentQueueCreateInfo.flags = 0;
  devicePresentQueueCreateInfo.queueFamilyIndex = g_presentQueueFamilyIndex;
  std::array<float, 1> presentQueuePriorities = {1.0f};
  devicePresentQueueCreateInfo.queueCount = presentQueuePriorities.size();
  devicePresentQueueCreateInfo.pQueuePriorities = presentQueuePriorities.data();

  std::array<VkDeviceQueueCreateInfo, 2> queueCreateInfos = {
      deviceGfxQueueCreateInfo, devicePresentQueueCreateInfo};

  VkDeviceCreateInfo deviceCreateInfo{};
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.pNext = nullptr;
  deviceCreateInfo.flags = 0;
  deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
  deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
  deviceCreateInfo.enabledLayerCount = REQUIRED_LAYERS.size();
  deviceCreateInfo.ppEnabledLayerNames = REQUIRED_LAYERS.data();
  deviceCreateInfo.enabledExtensionCount = REQUIRED_DEVICE_EXTENSIONS.size();
  deviceCreateInfo.ppEnabledExtensionNames = REQUIRED_DEVICE_EXTENSIONS.data();
  VkPhysicalDeviceFeatures features{};
  deviceCreateInfo.pEnabledFeatures = &features;

  {
    VkResult r =
        vkCreateDevice(g_physicalDevice, &deviceCreateInfo, nullptr, &g_device);
    if (r != VK_SUCCESS) {
      throw std::runtime_error("Failed to create logical device.");
    }
  }

  vkGetDeviceQueue(g_device, g_gfxQueueFamilyIndex, 0, &g_gfxQueue);
  vkGetDeviceQueue(g_device, g_presentQueueFamilyIndex, 0, &g_presentQueue);
}

static bool checkDeviceExtensionSupport(
    const std::vector<VkExtensionProperties>& deviceExtensions) {
  for (const auto& requiredExtensionName : REQUIRED_DEVICE_EXTENSIONS) {
    bool supported = std::ranges::any_of(
        deviceExtensions, [&](const VkExtensionProperties& props) {
          return props.extensionName == requiredExtensionName;
        });
    if (!supported) {
      return false;
    }
  }
  return true;
}

static std::vector<VkExtensionProperties> getDeviceExtensions(
    VkPhysicalDevice physicalDevice) {
  uint32_t count;
  vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count,
                                       nullptr);
  std::vector<VkExtensionProperties> extensions(count);
  vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count,
                                       extensions.data());
  return extensions;
}

static VkSurfaceCapabilitiesKHR getSurfaceCapabilities(VkPhysicalDevice physicalDevice) {
  VkSurfaceCapabilitiesKHR caps;
  VkResult r = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice,
                                                         g_surface, &caps);
  if (r != VK_SUCCESS) {
    throw std::runtime_error("Failed to fetch device surface capabilities");
  }
  return caps;
}

static std::vector<VkSurfaceFormatKHR> getSurfaceFormats(
    VkPhysicalDevice physicalDevice) {
  uint32_t count;
  {
    VkResult r = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, g_surface,
                                                      &count, nullptr);
    if (r != VK_SUCCESS) {
      throw std::runtime_error("Failed to fetch device surface formats");
    }
  }
  std::vector<VkSurfaceFormatKHR> formats(count);
  {
    VkResult r = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, g_surface,
                                                      &count, formats.data());
    if (r != VK_SUCCESS) {
      throw std::runtime_error("Failed to fetch device surface formats");
    }
  }
  return formats;
}

static std::vector<VkPresentModeKHR> getSurfacePresentModes(
    VkPhysicalDevice physicalDevice) {
  uint32_t count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, g_surface, &count,
                                            nullptr);
  std::vector<VkPresentModeKHR> presentModes(count);
  vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, g_surface, &count,
                                            presentModes.data());
  return presentModes;
}

static VkSurfaceFormatKHR selectSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& supportedFormats) {
  for (const auto& format : supportedFormats) {
    if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
        format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return format;
    }
  }
  throw std::runtime_error("Failed to select surface format");
}

static VkPresentModeKHR selectPresentMode(
    const std::vector<VkPresentModeKHR>& supportedPresentModes) {
  for (const auto& presentMode : supportedPresentModes) {
    if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return presentMode;
    }
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D selectSwapExtend(const VkSurfaceCapabilitiesKHR& caps) {
  if (caps.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    return caps.currentExtent;
  } else {
    int width, height;
    glfwGetFramebufferSize(strobe::glfw::g_window, &width, &height);
    VkExtent2D actualExtent = {static_cast<uint32_t>(width),
                               static_cast<uint32_t>(height)};
    actualExtent.width =
        std::clamp(actualExtent.width, caps.minImageExtent.width,
                   caps.maxImageExtent.width);
    actualExtent.height =
        std::clamp(actualExtent.height, caps.minImageExtent.height,
                   caps.maxImageExtent.height);
    return actualExtent;
  }
}

static void initSwapchain() {
  auto formats = getSurfaceFormats(g_physicalDevice);
  auto presentModes = getSurfacePresentModes(g_physicalDevice);
  auto caps = getSurfaceCapabilities(g_physicalDevice);

  VkSurfaceFormatKHR format = selectSurfaceFormat(formats);
  g_swapchainFormat = format.format;
  VkPresentModeKHR presentMode = selectPresentMode(presentModes);
  VkExtent2D extent = selectSwapExtend(caps);

  uint32_t imageCount = caps.minImageCount + 1;

  if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount) {
    imageCount = caps.maxImageCount;
  }
  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = g_surface;
  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = format.format;
  createInfo.imageColorSpace = format.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  std::array<uint32_t, 2> familyIndicies = {g_gfxQueueFamilyIndex,
                                            g_presentQueueFamilyIndex};

  if (g_gfxQueueFamilyIndex != g_presentQueueFamilyIndex) {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = familyIndicies.size();
    createInfo.pQueueFamilyIndices = familyIndicies.data();
  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;
  }
  createInfo.preTransform = caps.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;
  createInfo.oldSwapchain = VK_NULL_HANDLE;

  if (vkCreateSwapchainKHR(g_device, &createInfo, nullptr, &g_swapchain)) {
    throw std::runtime_error("Failed to create swapchain.");
  }
}

static std::vector<VkImage> getSwapchainImages() {
  uint32_t count;
  vkGetSwapchainImagesKHR(g_device, g_swapchain, &count, nullptr);
  std::vector<VkImage> images(count);
  vkGetSwapchainImagesKHR(g_device, g_swapchain, &count, images.data());
  return images;
}

static void initPhysicalDevice() {
  auto physicalDevices = getPhysicalDevices();
  logPhysicalDevices(physicalDevices);
  g_physicalDevice = selectPhysicalDevice(physicalDevices);
}
