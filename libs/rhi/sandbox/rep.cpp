#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>

static GLFWwindow *g_window = nullptr;
static VkInstance g_instance = VK_NULL_HANDLE;
static VkDebugUtilsMessengerEXT g_debugMessenger = VK_NULL_HANDLE;
static VkPhysicalDevice g_physicalDevice = VK_NULL_HANDLE;
static VkDevice g_device = VK_NULL_HANDLE;
static VkQueue g_queue = VK_NULL_HANDLE;
static uint32_t g_queueFamily = 0;
static VkSurfaceKHR g_surface = VK_NULL_HANDLE;
static VkSwapchainKHR g_swapchain = VK_NULL_HANDLE;
static VkCommandPool g_commandPool = VK_NULL_HANDLE;
static std::vector<VkImage> g_images;

struct Frame {
  uint32_t imageIndex = UINT32_MAX;
  VkSemaphore imageAvailable = VK_NULL_HANDLE;
  VkSemaphore presentReady = VK_NULL_HANDLE;
  VkFence presentFence = VK_NULL_HANDLE;
  VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
};

static std::vector<Frame> g_frames;

static void create_window() {
  if (!glfwInit()) {
    std::fprintf(stderr, "glfwInit failed\n");
    std::abort();
  }
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  GLFWwindow *window = glfwCreateWindow(800, 600, "FLOATING", nullptr, nullptr);
  if (!window) {
    std::fprintf(stderr, "glfwCreateWindow failed\n");
    std::abort();
  }
  g_window = window;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
               VkDebugUtilsMessageTypeFlagsEXT,
               const VkDebugUtilsMessengerCallbackDataEXT *data, void *) {
  const char *prefix = "VALIDATION";
  if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    prefix = "VALIDATION ERROR";
  else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    prefix = "VALIDATION WARNING";
  std::fprintf(stderr, "[%s] %s\n", prefix, data->pMessage);
  return VK_FALSE;
}

static VkDebugUtilsMessengerCreateInfoEXT debug_messenger_info() {
  return {
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
      .pNext = nullptr,
      .flags = 0,
      .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
      .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
      .pfnUserCallback = debug_callback,
      .pUserData = nullptr,
  };
}

static void create_instance() {
  VkApplicationInfo app{
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pNext = nullptr,
      .pApplicationName = "present-fence-mre",
      .applicationVersion = 1,
      .pEngineName = "none",
      .engineVersion = 1,
      .apiVersion = VK_API_VERSION_1_3,
  };

  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions =
      glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
  std::vector<const char *> extensions(glfwExtensions,
                                       glfwExtensions + glfwExtensionCount);
  extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  extensions.push_back(VK_KHR_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
  extensions.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
  const char *layers[] = {
      "VK_LAYER_KHRONOS_validation",
  };
  VkDebugUtilsMessengerCreateInfoEXT debugInfo = debug_messenger_info();
  VkInstanceCreateInfo info{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext = &debugInfo,
      .flags = 0,
      .pApplicationInfo = &app,
      .enabledLayerCount = 1,
      .ppEnabledLayerNames = layers,
      .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
      .ppEnabledExtensionNames = extensions.data(),
  };
  vkCreateInstance(&info, nullptr, &g_instance);
  auto createDebugMessenger =
      reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
          vkGetInstanceProcAddr(g_instance, "vkCreateDebugUtilsMessengerEXT"));
  VkDebugUtilsMessengerCreateInfoEXT messengerInfo = debug_messenger_info();
  createDebugMessenger(g_instance, &messengerInfo, nullptr, &g_debugMessenger);
}

static void create_surface() {
  const VkResult result =
      glfwCreateWindowSurface(g_instance, g_window, nullptr, &g_surface);
  if (result != VK_SUCCESS) {
    std::fprintf(stderr, "glfwCreateWindowSurface failed: %d\n",
                 static_cast<int>(result));
    std::abort();
  }
}

static void pick_device_and_queue() {
  uint32_t physicalDeviceCount = 0;
  vkEnumeratePhysicalDevices(g_instance, &physicalDeviceCount, nullptr);
  std::vector<VkPhysicalDevice> devices(physicalDeviceCount);
  vkEnumeratePhysicalDevices(g_instance, &physicalDeviceCount, devices.data());
  for (VkPhysicalDevice device : devices) {
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
    std::vector<VkQueueFamilyProperties> families(count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());
    for (uint32_t i = 0; i < count; ++i) {
      VkBool32 presentSupport = VK_FALSE;
      vkGetPhysicalDeviceSurfaceSupportKHR(device, i, g_surface,
                                           &presentSupport);
      if ((families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && presentSupport) {
        g_physicalDevice = device;
        g_queueFamily = i;
        return;
      }
    }
  }
  std::fprintf(stderr, "No graphics+present queue found\n");
  std::abort();
}

static void create_device() {
  float priority = 1.0f;
  VkDeviceQueueCreateInfo queueInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .queueFamilyIndex = g_queueFamily,
      .queueCount = 1,
      .pQueuePriorities = &priority,
  };
  VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR maintenanceFeature{
      .sType =
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_KHR,
      .pNext = nullptr,
      .swapchainMaintenance1 = VK_TRUE,
  };
  const char *extensions[] = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
      VK_KHR_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME,
  };
  VkDeviceCreateInfo info{
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext = &maintenanceFeature,
      .flags = 0,
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &queueInfo,
      .enabledLayerCount = 0,
      .ppEnabledLayerNames = nullptr,
      .enabledExtensionCount = 2,
      .ppEnabledExtensionNames = extensions,
      .pEnabledFeatures = nullptr,
  };
  vkCreateDevice(g_physicalDevice, &info, nullptr, &g_device);
  vkGetDeviceQueue(g_device, g_queueFamily, 0, &g_queue);
}

static void create_cmdpool() {
  VkCommandPoolCreateInfo poolInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .pNext = nullptr,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = g_queueFamily,
  };
  vkCreateCommandPool(g_device, &poolInfo, nullptr, &g_commandPool);
}

static VkSemaphore create_semaphore() {
  VkSemaphoreCreateInfo info{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
  };
  VkSemaphore semaphore;
  vkCreateSemaphore(g_device, &info, nullptr, &semaphore);
  return semaphore;
}

static VkFence create_fence() {
  VkFenceCreateInfo info{
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
  };
  VkFence fence;
  vkCreateFence(g_device, &info, nullptr, &fence);
  return fence;
}

static void cleanup() {
  vkDeviceWaitIdle(g_device);
  for (Frame &frame : g_frames) {
    vkDestroyFence(g_device, frame.presentFence, nullptr);
    vkDestroySemaphore(g_device, frame.presentReady, nullptr);
    vkDestroySemaphore(g_device, frame.imageAvailable, nullptr);
  }
  vkDestroyCommandPool(g_device, g_commandPool, nullptr);
  vkDestroySwapchainKHR(g_device, g_swapchain, nullptr);
  vkDestroyDevice(g_device, nullptr);
  vkDestroySurfaceKHR(g_instance, g_surface, nullptr);
  auto destroyDebugMessenger =
      reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
          vkGetInstanceProcAddr(g_instance, "vkDestroyDebugUtilsMessengerEXT"));
  destroyDebugMessenger(g_instance, g_debugMessenger, nullptr);
  vkDestroyInstance(g_instance, nullptr);
  glfwDestroyWindow(g_window);
  glfwTerminate();
}

static VkExtent2D choose_extent(GLFWwindow *window,
                                const VkSurfaceCapabilitiesKHR &caps) {
  if (caps.currentExtent.width != UINT32_MAX) {
    return caps.currentExtent;
  }
  int width;
  int height;
  glfwGetFramebufferSize(window, &width, &height);
  return {
      .width = std::clamp(static_cast<uint32_t>(width),
                          caps.minImageExtent.width, caps.maxImageExtent.width),
      .height =
          std::clamp(static_cast<uint32_t>(height), caps.minImageExtent.height,
                     caps.maxImageExtent.height),
  };
}

static void submit_undefined_to_present_image_transition(Frame &frame) {
  vkResetCommandBuffer(frame.commandBuffer, 0);

  VkCommandBufferBeginInfo begin{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .pNext = nullptr,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
      .pInheritanceInfo = nullptr,
  };

  vkBeginCommandBuffer(frame.commandBuffer, &begin);

  VkImageMemoryBarrier barrier{
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .pNext = nullptr,
      .srcAccessMask = 0,
      .dstAccessMask = 0,
      .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = g_images[frame.imageIndex],
      .subresourceRange =
          {
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .baseMipLevel = 0,
              .levelCount = 1,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
  };

  vkCmdPipelineBarrier(frame.commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                       VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0,
                       nullptr, 1, &barrier);

  vkEndCommandBuffer(frame.commandBuffer);

  VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

  VkSubmitInfo submit{
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .pNext = nullptr,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &frame.imageAvailable,
      .pWaitDstStageMask = &waitStage,
      .commandBufferCount = 1,
      .pCommandBuffers = &frame.commandBuffer,
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = &frame.presentReady,
  };

  vkQueueSubmit(g_queue, 1, &submit, VK_NULL_HANDLE);
}

int main() {
  create_window();
  create_instance();
  create_surface();
  pick_device_and_queue();
  create_device();
  create_cmdpool();

  // ==== create swapchain.
  VkSurfaceCapabilitiesKHR caps;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_physicalDevice, g_surface, &caps);
  uint32_t formatCount = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(g_physicalDevice, g_surface,
                                       &formatCount, nullptr);
  std::vector<VkSurfaceFormatKHR> formats(formatCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(g_physicalDevice, g_surface,
                                       &formatCount, formats.data());
  VkSurfaceFormatKHR format = formats[0];
  uint32_t requestedImageCount = caps.minImageCount + 4;
  if (caps.maxImageCount != 0) {
    requestedImageCount = std::min(requestedImageCount, caps.maxImageCount);
  }
  VkExtent2D extent = choose_extent(g_window, caps);
  VkSwapchainCreateInfoKHR swapchainInfo{
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .pNext = nullptr,
      .flags = 0,
      .surface = g_surface,
      .minImageCount = requestedImageCount,
      .imageFormat = format.format,
      .imageColorSpace = format.colorSpace,
      .imageExtent = extent,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
      .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = nullptr,
      .preTransform = caps.currentTransform,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR,
      .clipped = VK_TRUE,
      .oldSwapchain = VK_NULL_HANDLE,
  };
  vkCreateSwapchainKHR(g_device, &swapchainInfo, nullptr, &g_swapchain);

  // === Get swapchain images ====
  uint32_t imageCount = 0;
  vkGetSwapchainImagesKHR(g_device, g_swapchain, &imageCount, nullptr);
  g_images.resize(imageCount);
  vkGetSwapchainImagesKHR(g_device, g_swapchain, &imageCount, g_images.data());

  // ==== Initalize Frames In Flight =====
  const uint32_t batchSize = imageCount - caps.minImageCount + 1;
  g_frames.resize(batchSize);
  std::vector<VkCommandBuffer> commandBuffers(batchSize);
  VkCommandBufferAllocateInfo allocateInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .pNext = nullptr,
      .commandPool = g_commandPool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = batchSize,
  };
  vkAllocateCommandBuffers(g_device, &allocateInfo, commandBuffers.data());
  for (uint32_t i = 0; i < batchSize; ++i) {
    Frame &frame = g_frames[i];
    frame.commandBuffer = commandBuffers[i];
    frame.imageAvailable = create_semaphore();
    frame.presentReady = create_semaphore();
    frame.presentFence = create_fence();
  }

  std::printf("resize your window please\n");
  while (!glfwWindowShouldClose(g_window)) {
    glfwPollEvents();
    uint32_t acquiredCount = 0;

    // 1. Acquire N swapchain images
    bool acquireOutOfDate = false;
    for (uint32_t i = 0; i < batchSize; ++i) {
      Frame &frame = g_frames[i];
      const VkResult acquireResult = vkAcquireNextImageKHR(
          g_device, g_swapchain, UINT64_MAX, frame.imageAvailable,
          VK_NULL_HANDLE, &frame.imageIndex);
      if (acquireResult == VK_SUCCESS || acquireResult == VK_SUBOPTIMAL_KHR) {
        ++acquiredCount;
        submit_undefined_to_present_image_transition(frame);
        continue;
      }

      if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR) {
        acquireOutOfDate = true;
        break;
      }
    }
    if (acquiredCount == 0) {
      break; // sanity check
    }

    // wait for presentReady semaphores.
    vkQueueWaitIdle(g_queue);

    // I only run into a unsignaled fence if the window resize appears here!
    // (i.e. swapchain becomes out of date)
    glfwPollEvents();

    // 2. present all acquired images, one after another!
    bool presentOutOfDate = false;
    for (uint32_t i = 0; i < acquiredCount; ++i) {
      Frame &frame = g_frames[i];
      VkSwapchainPresentFenceInfoKHR fenceInfo{
          .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_FENCE_INFO_KHR,
          .pNext = nullptr,
          .swapchainCount = 1,
          .pFences = &frame.presentFence,
      };
      VkPresentInfoKHR presentInfo{
          .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
          .pNext = &fenceInfo,
          .waitSemaphoreCount = 1,
          .pWaitSemaphores = &frame.presentReady,
          .swapchainCount = 1,
          .pSwapchains = &g_swapchain,
          .pImageIndices = &frame.imageIndex,
          .pResults = nullptr,
      };
      VkResult presentResult = vkQueuePresentKHR(g_queue, &presentInfo);

      if (presentResult == VK_ERROR_OUT_OF_DATE_KHR) {
        // NOTE: out of date here is obviously fine, but as
        // far as i read the spec, it should still signal the
        // presentation fence.
        presentOutOfDate = true;
      } else if (presentResult == VK_SUBOPTIMAL_KHR) {
        // can't reproduce suboptimal, so i don't actually
        // know if there is also a bug on this path.
        // - by spec should be treated the same as out of date.
        presentOutOfDate = true;
      } else if (presentResult == VK_SUCCESS) {
      } else {
        std::fprintf(stderr, "vkQueuePresentKHR failed: %d\n",
                     static_cast<int>(presentResult));

        std::abort();
      }
    }

    // 3. Wait for all presentation fences.
    std::vector<VkFence> fences;
    fences.reserve(acquiredCount);
    for (uint32_t i = 0; i < acquiredCount; ++i) {
      fences.push_back(g_frames[i].presentFence);
    }
    if (presentOutOfDate) {
      std::printf("Present returned OUT_OF_DATE. "
                  "Waiting for %u present fences...\n",
                  acquiredCount);
      std::fflush(stdout);
    }

    // This sometimes blocks indefinitely, which as far as i
    // understand should not happen because out of date presents
    // should still signal the presentation fence.
    //
    // What i observed is that the first present signals it's fence
    // properly and then the second call to present after the
    // first has returned out of date doesn't signal the fence,
    // even if it returned out of date.
    //
    // NOTE: Although not shown in the example i observed the same
    // behavior with vkGetFenceStatus, where it simply doesn't get
    // signaled.
    //
    // NOTE: doesn't matter if it's one batched wait or
    // many single fence wait's!
    vkWaitForFences(g_device, acquiredCount, fences.data(), VK_TRUE,
                    UINT64_MAX);
    if (presentOutOfDate) {
      // everything worked, just sometimes one has to retry it
      // a couple of times to get the resize to happen between
      // acquire and present.
      std::printf("All present fences signaled.\n");
      break;
    }

    vkResetFences(g_device, acquiredCount, fences.data());
    if (acquireOutOfDate) {
      break;
    }
  }
  cleanup();
  return 0;
}
