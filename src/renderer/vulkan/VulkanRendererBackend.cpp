#include "renderer/vulkan/VulkanRendererBackend.hpp"

#include <cassert>
#include <iostream>

#include "window/glfw/GlfwWindowBackend.hpp"

namespace sge::vulkan {

#define GFX_QUEUE_IDENT 0
#define TRANSFER_QUEUE_IDENT 1
#define COMPUTE_QUEUE_IDENT 2

static const u32 MIN_GRAPHICS_QUEUES = 1;
static const u32 MAX_GRAPHICS_QUEUES = 2;
static const u32 MIN_TRANSFER_QUEUES = 0;
static const u32 MAX_TRANSFER_QUEUES = 1;
static const u32 MIN_COMPUTE_QUEUES = 0;
static const u32 MAX_COMPUTE_QUEUES = 0;

VulkanRendererBackend::VulkanRendererBackend(
    std::string application_name, std::tuple<int, int, int> application_version,
    std::string engine_name, std::tuple<int, int, int> engine_version,
    const Window &window) {
  //
  VkApplicationInfo appInfo;
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pNext = NULL;
  appInfo.pApplicationName = application_name.c_str();
  appInfo.applicationVersion = VK_MAKE_VERSION(
      std::get<0>(application_version), std::get<1>(application_version),
      std::get<2>(application_version));
  appInfo.pEngineName = engine_name.c_str();
  appInfo.engineVersion =
      VK_MAKE_VERSION(std::get<0>(engine_version), std::get<1>(engine_version),
                      std::get<2>(engine_version));
  appInfo.apiVersion = VK_MAKE_VERSION(1, 1, 0);

  uint32_t n_layersProperties;
  VkResult result =
      vkEnumerateInstanceLayerProperties(&n_layersProperties, nullptr);
  ASSERT_VKRESULT(result);

  std::vector<VkLayerProperties> layerProperties(n_layersProperties);
  result = vkEnumerateInstanceLayerProperties(&n_layersProperties,
                                              layerProperties.data());
  ASSERT_VKRESULT(result);

  std::vector<const char *> desiredLayers = {"VK_LAYER_KHRONOS_validation"};
  std::vector<const char *> avaiableLayers;
  for (VkLayerProperties layerProperty : layerProperties) {
    for (const char *desired : desiredLayers) {
      if (std::strcmp(layerProperty.layerName, desired) == 0) {
        avaiableLayers.push_back(desired);
      }
    }
  }

  uint32_t n_extentionProperties;
  result = vkEnumerateInstanceExtensionProperties(
      nullptr, &n_extentionProperties, nullptr);
  ASSERT_VKRESULT(result);
  std::vector<VkExtensionProperties> extentionProperties(n_extentionProperties);
  result = vkEnumerateInstanceExtensionProperties(
      nullptr, &n_extentionProperties, extentionProperties.data());
  ASSERT_VKRESULT(result);

  std::vector<const char *> desiredExtentions = {VK_KHR_SURFACE_EXTENSION_NAME};
  // fetch glfw extentions.
  std::vector<std::string> window_extentions =
      [&]() -> std::vector<std::string> {
    //
    switch (window.getBackendAPI()) {
      case GLFW_WINDOW_BACKEND:
        uint32_t count;
        const char **glfw_ext = glfwGetRequiredInstanceExtensions(&count);
        std::vector<std::string> ext(count);
        for (u32 i = 0; i < count; i++) {
          std::string str = glfw_ext[i];
          ext[i] = glfw_ext[i];
        }
        return ext;
    }
    return {};
  }();

  for (u64 i = 0; i < window_extentions.size(); i++) {
    for (const char *desired : desiredExtentions) {
      if (std::strcmp(desired, window_extentions[i].c_str())) {
        desiredExtentions.push_back(window_extentions[i].c_str());
        break;
      }
    }
  }

  std::vector<const char *> avaiableExtentions;
  for (VkExtensionProperties &extentionProperties : extentionProperties) {
    for (const char *desired : desiredExtentions) {
      if (std::strcmp(extentionProperties.extensionName, desired) == 0) {
        avaiableExtentions.push_back(desired);
      }
    }
  }

  VkInstanceCreateInfo instanceInfo;
  instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instanceInfo.pNext = nullptr;
  instanceInfo.flags = 0;
  instanceInfo.pApplicationInfo = &appInfo;
  instanceInfo.enabledLayerCount = avaiableLayers.size();
  instanceInfo.ppEnabledLayerNames = avaiableLayers.data();
  instanceInfo.enabledExtensionCount = avaiableExtentions.size();
  instanceInfo.ppEnabledExtensionNames = desiredExtentions.data();

  result = vkCreateInstance(&instanceInfo, nullptr, &m_instance);
  ASSERT_VKRESULT(result);

  uint32_t n_physicalDevices;
  result = vkEnumeratePhysicalDevices(m_instance, &n_physicalDevices, nullptr);
  ASSERT_VKRESULT(result);

  std::vector<VkPhysicalDevice> physicalDevices(n_physicalDevices);
  result = vkEnumeratePhysicalDevices(m_instance, &n_physicalDevices,
                                      physicalDevices.data());
  ASSERT_VKRESULT(result);

  u32 max_score = 0;
  VkPhysicalDevice selectedPhysicalDevice = nullptr;
  for (const VkPhysicalDevice &physicalDevice : physicalDevices) {
    u32 score = 0;

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      score += 1000;
    }

    uint64_t vram_size = 0;
    VkPhysicalDeviceMemoryProperties physicalMemoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice,
                                        &physicalMemoryProperties);
    VkMemoryHeap *pHeaps = physicalMemoryProperties.memoryHeaps;
    std::vector<VkMemoryHeap> heaps = std::vector<VkMemoryHeap>(
        pHeaps, pHeaps + physicalMemoryProperties.memoryHeapCount);
    for (const VkMemoryHeap &heap : heaps) {
      if (heap.flags & VkMemoryHeapFlagBits::VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
        vram_size += heap.size;
      }
    }
    score += vram_size;
    if (score > max_score) {
      max_score = score;
      selectedPhysicalDevice = physicalDevice;
    }
  }
  assert(selectedPhysicalDevice);  // raise if nullptr

  // TODO select a the queue with a bit more BRAIN !!!
  // i want one queue for transfer one for rendering and one for computes if
  // avaiable.
  uint32_t n_queue_family_properties;
  vkGetPhysicalDeviceQueueFamilyProperties(selectedPhysicalDevice,
                                           &n_queue_family_properties, nullptr);
  std::vector<VkQueueFamilyProperties> queue_family_properties(
      n_queue_family_properties);
  vkGetPhysicalDeviceQueueFamilyProperties(selectedPhysicalDevice,
                                           &n_queue_family_properties,
                                           queue_family_properties.data());
  std::optional<std::pair<u32, u32>> graphics_queue_family_index;
  std::optional<std::pair<u32, u32>> transfer_queue_family_index;
  std::optional<std::pair<u32, u32>> compute_queue_family_index;
  for (u32 index = 0; index < queue_family_properties.size(); index++) {
    if (queue_family_properties[index].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      graphics_queue_family_index = {index,
                                     queue_family_properties[index].queueCount};
    } else if (queue_family_properties[index].queueFlags &
               VK_QUEUE_TRANSFER_BIT) {
      transfer_queue_family_index = {index,
                                     queue_family_properties[index].queueCount};
    } else if (queue_family_properties[index].queueFlags &
               VK_QUEUE_COMPUTE_BIT) {
      compute_queue_family_index = {index,
                                    queue_family_properties[index].queueCount};
    }
  }

  const auto queueCreateInfoConstructor =
      [](std::optional<std::pair<u32, u32>> &family, std::string queue_name,
         u32 min, u32 max, std::vector<VkDeviceQueueCreateInfo> &out,
         std::vector<float> &prios) -> void {
    family->second = std::min(family->second, max);
    if (family.has_value() && family->second > 0) {
      if (family->second < min) {
        std::string err = "not enouth";
        err.append(queue_name);
        err.append(" queues avaiable");
        throw std::runtime_error(err);
      }
      prios.resize(family->second);
      for (u32 i = 0; i < prios.size(); i++) {
        prios[i] = 1.0f;
      }
      VkDeviceQueueCreateInfo queueCreateInfo;
      queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfo.pNext = nullptr;
      queueCreateInfo.flags = 0;
      queueCreateInfo.queueFamilyIndex = family->first;
      queueCreateInfo.queueCount = family->second;
      queueCreateInfo.pQueuePriorities = prios.data();
      out.push_back(queueCreateInfo);
    } else {
      if (min != 0) {
        std::string err = "no ";
        err.append(queue_name);
        err.append(" queue family avaiable");
        throw std::runtime_error(err);
      }
    }
  };

  std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
  std::vector<float> gfx_prios;
  queueCreateInfoConstructor(graphics_queue_family_index, "graphics",
                             MIN_GRAPHICS_QUEUES, MAX_GRAPHICS_QUEUES,
                             queue_create_infos, gfx_prios);
  std::vector<float> transfer_prios;
  queueCreateInfoConstructor(transfer_queue_family_index, "transfer",
                             MIN_TRANSFER_QUEUES, MAX_TRANSFER_QUEUES,
                             queue_create_infos, transfer_prios);
  std::vector<float> compute_prios;
  queueCreateInfoConstructor(compute_queue_family_index, "compute",
                             MIN_COMPUTE_QUEUES, MAX_COMPUTE_QUEUES,
                             queue_create_infos, compute_prios);

  VkPhysicalDeviceFeatures usedFeatures = {};

  const std::vector<const char *> deviceExtentions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  VkDeviceCreateInfo deviceCreateInfo;
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.pNext = nullptr;
  deviceCreateInfo.flags = 0;
  deviceCreateInfo.queueCreateInfoCount = queue_create_infos.size();
  deviceCreateInfo.pQueueCreateInfos = queue_create_infos.data();
  deviceCreateInfo.enabledLayerCount = 0;
  deviceCreateInfo.ppEnabledLayerNames = nullptr;
  deviceCreateInfo.enabledExtensionCount = deviceExtentions.size();
  deviceCreateInfo.ppEnabledExtensionNames = deviceExtentions.data();
  deviceCreateInfo.pEnabledFeatures = &usedFeatures;

  result = vkCreateDevice(selectedPhysicalDevice, &deviceCreateInfo, nullptr,
                          &m_device);

  for (u32 i = 0; i < graphics_queue_family_index->second; i++) {
    m_graphics_queues.resize(i + 1);
    vkGetDeviceQueue(m_device, graphics_queue_family_index->first, i,
                     &m_graphics_queues[i]);
    m_queueIds.emplace_back(GFX_QUEUE_IDENT, i);
  }
  for (u32 i = 0; i < transfer_queue_family_index->second; i++) {
    m_transfer_queues.resize(i + 1);
    vkGetDeviceQueue(m_device, transfer_queue_family_index->first, i,
                     &m_transfer_queues[i]);
    m_queueIds.emplace_back(TRANSFER_QUEUE_IDENT, i);
  }
  for (u32 i = 0; i < compute_queue_family_index->second; i++) {
    m_compute_queues.resize(i + 1);
    vkGetDeviceQueue(m_device, compute_queue_family_index->first, i,
                     &m_compute_queues[i]);
    m_queueIds.emplace_back(COMPUTE_QUEUE_IDENT, i);
  }

  // create surface and swapchain.
  if (window.getBackendAPI() == GLFW_WINDOW_BACKEND) {
    const glfw::GlfwWindowBackend &backend =
        static_cast<const glfw::GlfwWindowBackend &>(window.getBackend());
    result = glfwCreateWindowSurface(
        m_instance, const_cast<GLFWwindow *>(backend.pointer()), nullptr,
        &m_surface);
    ASSERT_VKRESULT(result);
  } else
    throw std::runtime_error("unsupported window backend");

  VkSurfaceCapabilitiesKHR sufaceCapabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(selectedPhysicalDevice, m_surface,
                                            &sufaceCapabilities);
  uint32_t n_formats;
  vkGetPhysicalDeviceSurfaceFormatsKHR(selectedPhysicalDevice, m_surface,
                                       &n_formats, nullptr);
  std::vector<VkSurfaceFormatKHR> formats(n_formats);
  vkGetPhysicalDeviceSurfaceFormatsKHR(selectedPhysicalDevice, m_surface,
                                       &n_formats, formats.data());
  // check if VK_FORMAT_B8G8R8A8_UNORM is supported.
  bool supportsFormat = false;
  for (VkSurfaceFormatKHR format : formats) {
    if (format.format == SURFACE_COLOR_FORMAT) {
      supportsFormat = true;
    }
  }
  assert(supportsFormat);

  uint32_t n_presentationModes;
  result = vkGetPhysicalDeviceSurfacePresentModesKHR(
      selectedPhysicalDevice, m_surface, &n_presentationModes, nullptr);
  ASSERT_VKRESULT(result);
  std::vector<VkPresentModeKHR> presentationModes(n_presentationModes);
  result = vkGetPhysicalDeviceSurfacePresentModesKHR(
      selectedPhysicalDevice, m_surface, &n_presentationModes,
      presentationModes.data());
  ASSERT_VKRESULT(result);

  VkSwapchainCreateInfoKHR swapchainCreateInfo;
  swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchainCreateInfo.pNext = nullptr;
  swapchainCreateInfo.flags = 0;
  swapchainCreateInfo.surface = m_surface;
  swapchainCreateInfo.minImageCount = std::min(sufaceCapabilities.minImageCount,
                                               (uint32_t)MAX_SWAPCHAIN_IMAGES);
  swapchainCreateInfo.imageFormat = VulkanRendererBackend::SURFACE_COLOR_FORMAT;
  swapchainCreateInfo.imageColorSpace =
      VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;  // TODO check if valid.
  swapchainCreateInfo.imageExtent = {window.getWidth(), window.getHeight()};
  swapchainCreateInfo.imageArrayLayers = 1;
  swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  swapchainCreateInfo.queueFamilyIndexCount = 0;
  swapchainCreateInfo.pQueueFamilyIndices = nullptr;
  swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchainCreateInfo.presentMode =
      VK_PRESENT_MODE_FIFO_KHR;  // TODO try if MAILBOX WORKS.
  swapchainCreateInfo.clipped = VK_TRUE;
  swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

  VkBool32 surfaceSupport = false;
  result = vkGetPhysicalDeviceSurfaceSupportKHR(selectedPhysicalDevice, 0,
                                                m_surface, &surfaceSupport);
  ASSERT_VKRESULT(result);
  assert(surfaceSupport);
  result = vkCreateSwapchainKHR(m_device, &swapchainCreateInfo, nullptr,
                                &m_swapchain);
  ASSERT_VKRESULT(result);
  uint32_t n_swapchainImages;
  result = vkGetSwapchainImagesKHR(m_device, m_swapchain, &n_swapchainImages,
                                   nullptr);
  ASSERT_VKRESULT(result);
  std::vector<VkImage> swapchainImages(n_swapchainImages);
  result = vkGetSwapchainImagesKHR(m_device, m_swapchain, &n_swapchainImages,
                                   swapchainImages.data());
  ASSERT_VKRESULT(result);

  m_swapchainImageViews.resize(n_swapchainImages);
  for (unsigned int i = 0; i < n_swapchainImages; i++) {
    uint32_t imageViewId = createImageView(
        swapchainImages[i], VulkanRendererBackend::SURFACE_COLOR_FORMAT);
    m_swapchainImageViews.push_back(imageViewId);
  }
  m_rendergraph = std::make_unique<VulkanMasterRendergraph>(this);
}

VulkanRendererBackend::~VulkanRendererBackend() {
  std::vector<uint32_t> allocatedBufferIds;
  for (uint32_t i = 0; i < m_commandBuffers.size(); i++) {
    bool allocated = true;
    for (const uint32_t empty_id : m_emptySemaphoreIds) {
      if (empty_id == i) {
        allocated = false;
        break;
      }
    }
    if (allocated) allocatedBufferIds.push_back(i);
  }
  for (uint32_t i = 0; i < m_commandPools.size(); i++) {
    bool allocated = true;
    for (const uint32_t empty_id : m_emptySemaphoreIds) {
      if (empty_id == i) {
        allocated = false;
        break;
      }
    }
    if (allocated) destroyCommandPool(i);
  }
  for (uint32_t i = 0; i < m_semaphores.size(); i++) {
    bool allocated = true;
    for (const uint32_t empty_id : m_emptySemaphoreIds) {
      if (empty_id == i) {
        allocated = false;
        break;
      }
    }
    if (allocated) destroySemaphore(i);
  }
  for (uint32_t i = 0; i < m_pipelines.size(); i++) {
    bool allocated = true;
    for (const uint32_t empty_id : m_empty_pipeline_ids) {
      if (empty_id == i) {
        allocated = false;
        break;
      }
    }
    if (allocated) destroyPipeline(i);
  }
  for (uint32_t i = 0; i < m_framebuffers.size(); i++) {
    bool allocated = true;
    for (const uint32_t empty_id : m_emptyFramebufferIds) {
      if (empty_id == i) {
        allocated = false;
        break;
      }
    }
    if (allocated) destroyFramebuffer(i);
  }
  for (uint32_t i = 0; i < m_pipelineLayouts.size(); i++) {
    bool allocated = true;
    for (const uint32_t empty_id : m_emptyPipelineLayoutIds) {
      if (empty_id == i) {
        allocated = false;
        break;
      }
    }
    if (allocated) destroyPipelineLayout(i);
  }
  for (uint32_t i = 0; i < m_shaders.size(); i++) {
    bool allocated = true;
    for (const uint32_t empty_id : m_empty_shader_ids) {
      if (empty_id == i) {
        allocated = false;
        break;
      }
    }
    if (allocated) destroyShaderModule(i);
  }
  for (uint32_t i = 0; i < m_render_passes.size(); i++) {
    bool allocated = true;
    for (const uint32_t empty_id : m_empty_render_pass_ids) {
      if (empty_id == i) {
        allocated = false;
        break;
      }
    }

    if (allocated) destroyRenderPass(i);
  }
  for (uint32_t i = 0; i < m_imageViews.size(); i++) {
    bool allocated = true;
    for (const uint32_t empty_id : m_emptyImageViewIds) {
      if (empty_id == i) {
        allocated = false;
        break;
      }
    }
    if (allocated) destroyImageView(i);
  }
  vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
  vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
  vkDestroyDevice(m_device, nullptr);
  vkDestroyInstance(m_instance, nullptr);
}

void VulkanRendererBackend::beginFrame() {
  //
  m_rendergraph->beginFrame();
}

void VulkanRendererBackend::renderFrame() {
  //
  m_rendergraph->execute();
}

void VulkanRendererBackend::endFrame() {
  //
  m_rendergraph->endFrame();
}

uint32_t VulkanRendererBackend::createImageView(VkImage image,
                                                VkFormat format) {
  VkImageView imageView;
  VkImageViewCreateInfo imageViewCreateInfo;
  imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  imageViewCreateInfo.pNext = nullptr;
  imageViewCreateInfo.flags = 0;
  imageViewCreateInfo.image = image;
  imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  imageViewCreateInfo.format = format;
  imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
  imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
  imageViewCreateInfo.subresourceRange.levelCount = 1;
  // COMPLETLY USELESS =^)
  imageViewCreateInfo.subresourceRange.layerCount = 1;
  imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
  vkCreateImageView(m_device, &imageViewCreateInfo, nullptr, &imageView);

  if (m_emptyImageViewIds.empty()) {
    m_imageViews.push_back(imageView);
    return m_imageViews.size() - 1;
  } else {
    uint32_t id = m_emptyImageViewIds.back();
    m_emptyImageViewIds.pop_back();
    m_imageViews[id] = imageView;
    return id;
  }
}

void VulkanRendererBackend::destroyImageView(uint32_t imageViewId) {
  vkDestroyImageView(m_device, getImageViewById(imageViewId), nullptr);
  m_emptyImageViewIds.push_back(imageViewId);
}

uint32_t VulkanRendererBackend::createShaderModule(
    const std::vector<char> source_code) {
  // create vulkan shader module
  VkShaderModuleCreateInfo shaderCreateInfo;
  shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shaderCreateInfo.pNext = nullptr;
  shaderCreateInfo.flags = 0;
  shaderCreateInfo.codeSize = source_code.size();
  shaderCreateInfo.pCode =
      reinterpret_cast<const uint32_t *>(source_code.data());
  VkShaderModule shader_module;
  VkResult result = vkCreateShaderModule(m_device, &shaderCreateInfo, nullptr,
                                         &shader_module);
  ASSERT_VKRESULT(result);
  // select id for new shader.
  if (m_empty_shader_ids.empty()) {
    m_shaders.push_back(shader_module);
    return m_shaders.size() - 1;
  } else {
    uint32_t id = m_empty_shader_ids.back();
    m_empty_shader_ids.pop_back();
    m_shaders[id] = shader_module;
    return id;
  }
}

void VulkanRendererBackend::destroyShaderModule(uint32_t shader_module_id) {
  vkDestroyShaderModule(m_device, getShaderById(shader_module_id), nullptr);
  m_empty_shader_ids.push_back(shader_module_id);
}

uint32_t VulkanRendererBackend::createPipelineLayout() {
  //
  VkPipelineLayout layout;
  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
  pipelineLayoutCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutCreateInfo.pNext = nullptr;
  pipelineLayoutCreateInfo.flags = 0;
  pipelineLayoutCreateInfo.setLayoutCount = 0;
  pipelineLayoutCreateInfo.pSetLayouts = nullptr;
  pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
  pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

  VkResult result = vkCreatePipelineLayout(m_device, &pipelineLayoutCreateInfo,
                                           nullptr, &layout);
  ASSERT_VKRESULT(result);
  if (m_empty_pipeline_ids.empty()) {
    m_pipelineLayouts.push_back(layout);
    return m_pipelineLayouts.size() - 1;
  } else {
    uint32_t id = m_emptyPipelineLayoutIds.back();
    m_emptyPipelineLayoutIds.pop_back();
    m_pipelineLayouts[id] = layout;
    return id;
  }
}
void VulkanRendererBackend::destroyPipelineLayout(uint32_t pipelineLayoutId) {
  //
  vkDestroyPipelineLayout(m_device, getPipelineLayoutById(pipelineLayoutId),
                          nullptr);
  m_emptyPipelineLayoutIds.push_back(pipelineLayoutId);
}

void VulkanRendererBackend::bindPipeline(uint32_t pipelineId,
                                         uint32_t commandBufferId) {
  vkCmdBindPipeline(getCommandBufferById(commandBufferId),
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    getPipelineById(pipelineId));
}

uint32_t VulkanRendererBackend::createRenderPass(const VkFormat color_format) {
  VkRenderPass render_pass;
  VkResult result;

  VkAttachmentDescription attachmentDescription;
  attachmentDescription.flags = 0;
  attachmentDescription.format = color_format;
  attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
  attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference attachmentReference;
  attachmentReference.attachment = 0;
  attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpassDescription;
  subpassDescription.flags = 0;
  subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpassDescription.inputAttachmentCount = 0;
  subpassDescription.pInputAttachments = nullptr;
  subpassDescription.colorAttachmentCount = 1;
  subpassDescription.pColorAttachments = &attachmentReference;
  subpassDescription.pResolveAttachments = nullptr;
  subpassDescription.pDepthStencilAttachment = nullptr;
  subpassDescription.preserveAttachmentCount = 0;
  subpassDescription.pPreserveAttachments = nullptr;

  VkSubpassDependency subpassDependency;
  subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  subpassDependency.dstSubpass = 0;
  subpassDependency.srcStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependency.dstStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependency.srcAccessMask = 0;
  subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  subpassDependency.dependencyFlags = 0;

  VkRenderPassCreateInfo renderPassCreateInfo;
  renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassCreateInfo.pNext = nullptr;
  renderPassCreateInfo.flags = 0;
  renderPassCreateInfo.attachmentCount = 1;
  renderPassCreateInfo.pAttachments = &attachmentDescription;
  renderPassCreateInfo.subpassCount = 1;
  renderPassCreateInfo.pSubpasses = &subpassDescription;
  renderPassCreateInfo.dependencyCount = 1;
  renderPassCreateInfo.pDependencies = &subpassDependency;

  result = vkCreateRenderPass(m_device, &renderPassCreateInfo, nullptr,
                              &render_pass);
  ASSERT_VKRESULT(result);

  if (m_empty_render_pass_ids.empty()) {
    m_render_passes.push_back(render_pass);
    return m_render_passes.size() - 1;
  } else {
    uint32_t id = m_empty_render_pass_ids.back();
    m_empty_render_pass_ids.pop_back();
    m_render_passes[id] = render_pass;
    return id;
  }
}

void VulkanRendererBackend::beginRenderPass(uint32_t renderPassId,
                                            uint32_t framebufferId,
                                            uint32_t renderAreaWidth,
                                            uint32_t renderAreaHeight,
                                            uint32_t commandBufferId) {
  VkRenderPassBeginInfo renderPassBeginInfo;
  renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassBeginInfo.pNext = nullptr;
  renderPassBeginInfo.renderPass = getRenderPassById(renderPassId);
  renderPassBeginInfo.framebuffer = getFramebufferById(framebufferId);
  renderPassBeginInfo.renderArea.offset = {0, 0};
  renderPassBeginInfo.renderArea.extent = {renderAreaWidth, renderAreaHeight};

  VkClearValue clearValue;
  clearValue.color.float32[0] = 0;
  clearValue.color.float32[1] = 0;
  clearValue.color.float32[2] = 0;
  clearValue.color.float32[3] = 1;

  renderPassBeginInfo.clearValueCount = 1;
  renderPassBeginInfo.pClearValues = &clearValue;

  vkCmdBeginRenderPass(getCommandBufferById(commandBufferId),
                       &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanRendererBackend::endRenderPass(uint32_t commandBufferId) {
  vkCmdEndRenderPass(getCommandBufferById(commandBufferId));
}

void VulkanRendererBackend::destroyRenderPass(uint32_t renderPassId) {
  vkDestroyRenderPass(m_device, getRenderPassById(renderPassId), nullptr);
  m_empty_render_pass_ids.push_back(renderPassId);
}

uint32_t VulkanRendererBackend::createPipeline(
    uint32_t renderPassId, uint32_t pipelineLayoutId, uint32_t viewportWidth,
    uint32_t viewportHeight, std::optional<uint32_t> vertexShaderId,
    std::optional<uint32_t> fragmentShaderId) {
  VkPipeline pipeline;
  VkResult result;

  std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
  if (vertexShaderId.has_value()) {
    VkPipelineShaderStageCreateInfo shaderStageCreateInfoVert;
    shaderStageCreateInfoVert.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageCreateInfoVert.pNext = nullptr;
    shaderStageCreateInfoVert.flags = 0;
    shaderStageCreateInfoVert.stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStageCreateInfoVert.module = getShaderById(vertexShaderId.value());
    shaderStageCreateInfoVert.pName = "main";
    shaderStageCreateInfoVert.pSpecializationInfo = nullptr;
    shaderStages.push_back(shaderStageCreateInfoVert);
  }
  if (fragmentShaderId.has_value()) {
    VkPipelineShaderStageCreateInfo shaderStageCreateInfoFrag;
    shaderStageCreateInfoFrag.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageCreateInfoFrag.pNext = nullptr;
    shaderStageCreateInfoFrag.flags = 0;
    shaderStageCreateInfoFrag.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStageCreateInfoFrag.module = getShaderById(fragmentShaderId.value());
    shaderStageCreateInfoFrag.pName = "main";
    shaderStageCreateInfoFrag.pSpecializationInfo = nullptr;
    shaderStages.push_back(shaderStageCreateInfoFrag);
  }

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

  VkViewport viewport;
  viewport.x = 0;
  viewport.y = 0;
  viewport.width = viewportWidth;
  viewport.height = viewportHeight;
  viewport.minDepth = 0.0;
  viewport.maxDepth = 1.0;
  VkRect2D scissor;
  scissor.offset = {0, 0};
  scissor.extent = {viewportWidth, viewportHeight};
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
  pipelineCreateInfo.pStages = shaderStages.data();
  pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
  pipelineCreateInfo.pInputAssemblyState = &inputAssemlyCreateInfo;
  pipelineCreateInfo.pTessellationState = nullptr;
  pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
  pipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
  pipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
  pipelineCreateInfo.pDepthStencilState = nullptr;
  pipelineCreateInfo.pColorBlendState = &colorBlendCreateInfo;
  pipelineCreateInfo.pDynamicState = nullptr;
  pipelineCreateInfo.layout = getPipelineLayoutById(pipelineLayoutId);
  pipelineCreateInfo.renderPass = getRenderPassById(renderPassId);
  pipelineCreateInfo.subpass = 0;
  pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineCreateInfo.basePipelineIndex = -1;

  result = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1,
                                     &pipelineCreateInfo, nullptr, &pipeline);
  ASSERT_VKRESULT(result);
  //
  if (m_empty_pipeline_ids.empty()) {
    m_pipelines.push_back(pipeline);
    return m_pipelines.size() - 1;
  } else {
    uint32_t id = m_empty_pipeline_ids.back();
    m_empty_pipeline_ids.pop_back();
    m_pipelines[id] = pipeline;
    return id;
  }
}

void VulkanRendererBackend::destroyPipeline(uint32_t pipelineId) {
  vkDestroyPipeline(m_device, getPipelineById(pipelineId), nullptr);
  m_empty_pipeline_ids.push_back(pipelineId);
}

uint32_t VulkanRendererBackend::createFramebuffer(uint32_t renderPassId,
                                                  uint32_t imageViewId,
                                                  uint32_t width,
                                                  uint32_t height) {
  VkFramebuffer framebuffer;

  VkFramebufferCreateInfo framebufferCreateInfo;
  framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  framebufferCreateInfo.pNext = nullptr;
  framebufferCreateInfo.flags = 0;
  framebufferCreateInfo.renderPass = getRenderPassById(renderPassId);
  framebufferCreateInfo.attachmentCount = 1;
  framebufferCreateInfo.pAttachments = &getImageViewById(imageViewId);
  framebufferCreateInfo.width = width;
  framebufferCreateInfo.height = height;
  framebufferCreateInfo.layers = 1;
  VkResult result = vkCreateFramebuffer(m_device, &framebufferCreateInfo,
                                        nullptr, &framebuffer);
  ASSERT_VKRESULT(result);

  if (m_emptyFramebufferIds.empty()) {
    m_framebuffers.push_back(framebuffer);
    return m_framebuffers.size() - 1;
  } else {
    uint32_t id = m_emptyFramebufferIds.back();
    m_emptyFramebufferIds.pop_back();
    m_framebuffers[id] = framebuffer;
    return id;
  }
}

void VulkanRendererBackend::destroyFramebuffer(uint32_t framebufferId) {
  vkDestroyFramebuffer(m_device, getFramebufferById(framebufferId), nullptr);
  m_emptyFramebufferIds.push_back(framebufferId);
}

uint32_t VulkanRendererBackend::createCommandPool(uint32_t queueFamilyIndex) {
  VkCommandPool commandPool;
  VkCommandPoolCreateInfo commandPoolCreateInfo;
  commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  commandPoolCreateInfo.pNext = nullptr;
  commandPoolCreateInfo.flags = 0;
  commandPoolCreateInfo.queueFamilyIndex = 0;  // DOESN'T PROBABLY WORK =^(.
  VkResult result = vkCreateCommandPool(m_device, &commandPoolCreateInfo,
                                        nullptr, &commandPool);
  ASSERT_VKRESULT(result);
  if (m_emptyCommandPoolIds.empty()) {
    m_commandPools.push_back(commandPool);
    return m_commandPools.size() - 1;
  } else {
    uint32_t id = m_emptyCommandPoolIds.back();
    m_emptyCommandPoolIds.pop_back();
    m_commandPools[id] = commandPool;
    return id;
  }
}

void VulkanRendererBackend::destroyCommandPool(uint32_t commandPoolId) {
  vkDestroyCommandPool(m_device, getCommandPoolById(commandPoolId), nullptr);
  m_emptyCommandPoolIds.push_back(commandPoolId);
}

const std::vector<uint32_t> VulkanRendererBackend::allocateCommandBuffers(
    uint32_t commandPoolId, uint32_t count) {
  VkCommandBufferAllocateInfo commandBufferAllocateInfo;
  commandBufferAllocateInfo.sType =
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  commandBufferAllocateInfo.pNext = nullptr;
  commandBufferAllocateInfo.commandPool = getCommandPoolById(commandPoolId);
  commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  commandBufferAllocateInfo.commandBufferCount = count;

  std::vector<VkCommandBuffer> buffer(count);
  VkResult result = vkAllocateCommandBuffers(
      m_device, &commandBufferAllocateInfo, buffer.data());
  ASSERT_VKRESULT(result);
  std::vector<uint32_t> ids(count);
  for (uint32_t i = 0; i < count; i++) {
    if (m_emptyCommandBufferIds.empty()) {
      m_commandBuffers.push_back(buffer[i]);
      m_emptyCommandBufferIds.push_back(commandPoolId);
      ids[i] = m_commandBuffers.size() - 1;
    } else {
      ids[i] = m_emptyCommandBufferIds.back();
      m_emptyCommandBufferIds.pop_back();
      m_commandBuffers[ids[i]] = buffer[i];
      m_commandBufferIdToPoolId[ids[i]] = commandPoolId;
    }
  }
  return ids;
}

void VulkanRendererBackend::freeCommandBuffers(
    const std::vector<uint32_t> &commandBufferIds) {
  if (commandBufferIds.empty()) return;
  std::vector<VkCommandBuffer> buffers(commandBufferIds.size());
  std::optional<uint32_t> poolId;
  for (const uint32_t bufferId : commandBufferIds) {
    buffers.push_back(getCommandBufferById(bufferId));
    m_emptyCommandBufferIds.push_back(bufferId);
    if (!poolId.has_value()) {
      poolId = m_commandBufferIdToPoolId[bufferId];
    } else {
      assert(poolId == m_commandBufferIdToPoolId[bufferId]);
    }
  }
  vkFreeCommandBuffers(m_device, getCommandPoolById(poolId.value()),
                       buffers.size(), buffers.data());
}

void VulkanRendererBackend::beginCommandBuffer(uint32_t commandBufferId) {
  VkCommandBufferBeginInfo commandBufferBeginInfo;
  commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  commandBufferBeginInfo.pNext = nullptr;
  commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
  commandBufferBeginInfo.pInheritanceInfo = nullptr;

  VkResult result = vkBeginCommandBuffer(getCommandBufferById(commandBufferId),
                                         &commandBufferBeginInfo);
  ASSERT_VKRESULT(result);
}

void VulkanRendererBackend::endCommandBuffer(uint32_t commandBufferId) {
  VkResult result = vkEndCommandBuffer(getCommandBufferById(commandBufferId));
  ASSERT_VKRESULT(result);
}

uint32_t VulkanRendererBackend::createSemaphore() {
  VkSemaphore semaphore;
  VkSemaphoreCreateInfo semaphoreCreateInfo;
  semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  semaphoreCreateInfo.pNext = nullptr;
  semaphoreCreateInfo.flags = 0;
  VkResult result =
      vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &semaphore);
  ASSERT_VKRESULT(result);
  if (m_emptySemaphoreIds.empty()) {
    m_semaphores.push_back(semaphore);
    return m_semaphores.size() - 1;
  } else {
    uint32_t id = m_emptySemaphoreIds.back();
    m_emptySemaphoreIds.pop_back();
    m_semaphores[id] = semaphore;
    return id;
  }
}

void VulkanRendererBackend::destroySemaphore(uint32_t semaphoreId) {
  vkDestroySemaphore(m_device, getSemaphoreById(semaphoreId), nullptr);
  m_emptySemaphoreIds.push_back(semaphoreId);
}

void VulkanRendererBackend::drawCall(uint32_t vertexCount,
                                     uint32_t instanceCount,
                                     uint32_t commandBufferId) {
  vkCmdDraw(getCommandBufferById(commandBufferId), vertexCount, instanceCount,
            0, 0);
}
void VulkanRendererBackend::acquireNextSwapchainFrame(u32 signalSem) {
  VkResult result = vkAcquireNextImageKHR(
      m_device, m_swapchain, std::numeric_limits<uint64_t>::max(),
      getSemaphoreById(signalSem), VK_NULL_HANDLE, &m_swapchainFrameIndex);
  ASSERT_VKRESULT(result);
}

void VulkanRendererBackend::presentQueue(
    u32 queueId, const std::vector<u32> &waitSemaphoreIds) {
  VkPresentInfoKHR presentInfo;
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.pNext = nullptr;
  presentInfo.waitSemaphoreCount = waitSemaphoreIds.size();
  std::vector<VkSemaphore> waitSemaphores(waitSemaphoreIds.size());
  for (u32 i = 0; i < waitSemaphoreIds.size(); i++) {
    waitSemaphores[i] = getSemaphoreById(waitSemaphoreIds[i]);
  }
  presentInfo.pWaitSemaphores = waitSemaphores.data();
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &m_swapchain;
  presentInfo.pImageIndices = &m_swapchainFrameIndex;
  presentInfo.pResults = nullptr;
  VkResult result = vkQueuePresentKHR(getQueueById(queueId), &presentInfo);
  ASSERT_VKRESULT(result);
}

u32 VulkanRendererBackend::getAnyGraphicsQueue() {
  for (u32 i = 0; i < m_queueIds.size(); i++) {
    if (m_queueIds[i].first == GFX_QUEUE_IDENT) {
      return i;
    }
  }
  throw std::runtime_error("no graphics queue avaiable");
}
u32 VulkanRendererBackend::getAnyTransferQueue() {
  for (u32 i = 0; i < m_queueIds.size(); i++) {
    if (m_queueIds[i].first == TRANSFER_QUEUE_IDENT) {
      return i;
    }
  }
  throw std::runtime_error("no transfer queue avaiable");
}
u32 VulkanRendererBackend::getAnyComputeQueue() {
  for (u32 i = 0; i < m_queueIds.size(); i++) {
    if (m_queueIds[i].first == COMPUTE_QUEUE_IDENT) {
      return i;
    }
  }
  throw std::runtime_error("no compute queue avaiable");
}

VkImageView &VulkanRendererBackend::getImageViewById(
    const uint32_t imageViewId) {
  assert(imageViewId < m_imageViews.size());
  return m_imageViews[imageViewId];
}

VkPipelineLayout &VulkanRendererBackend::getPipelineLayoutById(
    const uint32_t pipelineLayoutId) {
  assert(pipelineLayoutId < m_pipelineLayouts.size());
  return m_pipelineLayouts[pipelineLayoutId];
}
VkPipeline &VulkanRendererBackend::getPipelineById(const uint32_t pipelineId) {
  assert(pipelineId < m_pipelines.size());
  return m_pipelines[pipelineId];
}
VkRenderPass &VulkanRendererBackend::getRenderPassById(
    const uint32_t renderPassId) {
  assert(renderPassId < m_render_passes.size());
  return m_render_passes[renderPassId];
}
VkShaderModule &VulkanRendererBackend::getShaderById(const uint32_t shaderId) {
  assert(shaderId < m_shaders.size());
  return m_shaders[shaderId];
}

VkFramebuffer &VulkanRendererBackend::getFramebufferById(
    const uint32_t framebufferId) {
  assert(framebufferId < m_framebuffers.size());
  return m_framebuffers[framebufferId];
}

VkCommandPool &VulkanRendererBackend::getCommandPoolById(
    const uint32_t commandPoolId) {
  assert(commandPoolId < m_commandPools.size());
  return m_commandPools[commandPoolId];
}

VkCommandBuffer &VulkanRendererBackend::getCommandBufferById(
    const uint32_t commandBufferId) {
  assert(commandBufferId < m_commandBuffers.size());
  return m_commandBuffers[commandBufferId];
}

VkSemaphore &VulkanRendererBackend::getSemaphoreById(
    const uint32_t semaphoreId) {
  assert(semaphoreId < m_semaphores.size());
  return m_semaphores[semaphoreId];
}

VkQueue &VulkanRendererBackend::getQueueById(const u32 queueId) {
  assert(queueId < m_queueIds.size());
  const u32 type = m_queueIds[queueId].first;
  const u32 index = m_queueIds[queueId].second;
  switch (type) {
    case GFX_QUEUE_IDENT:
      assert(index < m_graphics_queues.size());
      return m_graphics_queues[index];
    case TRANSFER_QUEUE_IDENT:
      assert(index < m_transfer_queues.size());
      return m_transfer_queues[index];
    case COMPUTE_QUEUE_IDENT:
      assert(index < m_compute_queues.size());
      return m_compute_queues[index];
  }
  throw std::runtime_error("how did we get here =^) !");
}

}  // namespace sge::vulkan
