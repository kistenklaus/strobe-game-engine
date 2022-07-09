#include "renderer/vulkan/VRendererBackend.hpp"

#include <cassert>
#include <iostream>
#include <regex>

#include "fileio/fileio.hpp"
#include "renderer/vulkan/VMasterRendergraph.hpp"
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

VRendererBackend::VRendererBackend(
    std::string &applicationName, std::tuple<u32, u32, u32> &applicationVersion,
    std::string &engineName, std::tuple<u32, u32, u32> &engineVersion,
    Window *window)
    : mp_window(window) {
  m_instance = createInstance(applicationName, applicationVersion, engineName,
                              engineVersion);
  m_device = createDevice();
  m_surface = createSurface();
  updateSwapchainSupport();
  createSwapchain();
  m_rootPass = new VMasterRendergraph(this);
}

VRendererBackend::~VRendererBackend() {
  waitDeviceIdle();
  m_rootPass->dispose();
  delete m_rootPass;

  for (vertex_buffer_t &vertexBuffer : m_vertexBuffers) {
    destroyVertexBuffer(vertexBuffer);
  }

  for (command_buffer_t &command_buffer : m_commandBuffers) {
    // freeing is propably not required if we destroy the pools
  }
  for (command_pool_t &command_pool : m_commandPools) {
    destroyCommandPool(command_pool);
  }
  for (semaphore_t &semaphore : m_semaphores) {
    destroySemaphore(semaphore);
  }
  for (pipeline_t &pipeline : m_pipelines) {
    destroyPipeline(pipeline);
  }

  for (pipeline_layout_t &pipelineLayout : m_pipelineLayouts) {
    destroyPipelineLayout(pipelineLayout);
  }
  for (shader_module_t &shaderModule : m_shaders) {
    destroyShaderModule(shaderModule);
  }
  for (framebuffer_t &framebuffer : m_framebuffers) {
    destroyFramebuffer(framebuffer);
  }
  for (renderpass_t &renderpass : m_renderpasses) {
    destroyRenderPass(renderpass);
  }
  for (imageview_t &imageview : m_imageViews) {
    destroyImageView(imageview);
  }
  destroySwapchain(m_swapchain);
  destroySurface(m_surface);
  destroyDevice(m_device);
  destroyInstance(m_instance);
  // vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
  // vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
  // vkDestroyDevice(m_device, nullptr);
  // vkDestroyInstance(m_instance, nullptr);
}

void VRendererBackend::recreateSwapchain() {
  //
  waitDeviceIdle();

  for (const imageview &imageview : m_swapchain.m_imageViewsHandles) {
    destroyImageView(imageview);
  }

  swapchain_t oldSwapchain = m_swapchain;
  createSwapchain();

  destroySwapchain(oldSwapchain);
}

VRendererBackend::instance_t VRendererBackend::createInstance(
    const std::string &applicationName,
    const std::tuple<u32, u32, u32> &applicationVersion,
    const std::string &engineName,
    const std::tuple<u32, u32, u32> &engineVersion) {
  VkApplicationInfo appInfo;
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pNext = NULL;
  appInfo.pApplicationName = applicationName.c_str();
  appInfo.applicationVersion = VK_MAKE_VERSION(std::get<0>(applicationVersion),
                                               std::get<1>(applicationVersion),
                                               std::get<2>(applicationVersion));
  appInfo.pEngineName = engineName.c_str();
  appInfo.engineVersion =
      VK_MAKE_VERSION(std::get<0>(engineVersion), std::get<1>(engineVersion),
                      std::get<2>(engineVersion));
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
    switch (mp_window->getBackendAPI()) {
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

  instance_t instance{};
  result = vkCreateInstance(&instanceInfo, nullptr, &instance.m_handle);
  ASSERT_VKRESULT(result);
  return instance;
}

VRendererBackend::device_t VRendererBackend::createDevice() {
  device_t device{};
  uint32_t n_physicalDevices;
  VkResult result = vkEnumeratePhysicalDevices(m_instance.m_handle,
                                               &n_physicalDevices, nullptr);
  ASSERT_VKRESULT(result);

  std::vector<VkPhysicalDevice> physicalDevices(n_physicalDevices);
  result = vkEnumeratePhysicalDevices(m_instance.m_handle, &n_physicalDevices,
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
  device.m_physicalDevice = selectedPhysicalDevice;

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
      device.m_gfxQueueFamilyIndex = index;
    } else if (queue_family_properties[index].queueFlags &
               VK_QUEUE_TRANSFER_BIT) {
      transfer_queue_family_index = {index,
                                     queue_family_properties[index].queueCount};
      device.m_transferQueueFamilyIndex = index;
    } else if (queue_family_properties[index].queueFlags &
               VK_QUEUE_COMPUTE_BIT) {
      compute_queue_family_index = {index,
                                    queue_family_properties[index].queueCount};
      device.m_computeQueueFamilyIndex = index;
    }
  }

  const auto queueCreateInfoConstructor =
      [](std::optional<std::pair<u32, u32>> &family, std::string queue_name,
         u32 min, u32 max, std::vector<VkDeviceQueueCreateInfo> &out,
         std::vector<float> &prios) -> u32 {
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
      return family->second;
    } else {
      if (min != 0) {
        std::string err = "no ";
        err.append(queue_name);
        err.append(" queue family avaiable");
        throw std::runtime_error(err);
      }
      return 0;
    }
  };

  std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
  std::vector<float> gfx_prios;
  const u32 graphicsQueueCount = queueCreateInfoConstructor(
      graphics_queue_family_index, "graphics", MIN_GRAPHICS_QUEUES,
      MAX_GRAPHICS_QUEUES, queue_create_infos, gfx_prios);
  std::vector<float> transfer_prios;
  const u32 transferQueueCount = queueCreateInfoConstructor(
      transfer_queue_family_index, "transfer", MIN_TRANSFER_QUEUES,
      MAX_TRANSFER_QUEUES, queue_create_infos, transfer_prios);
  std::vector<float> compute_prios;
  const u32 computeQueueCount = queueCreateInfoConstructor(
      compute_queue_family_index, "compute", MIN_COMPUTE_QUEUES,
      MAX_COMPUTE_QUEUES, queue_create_infos, compute_prios);

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
                          &device.m_handle);

  for (u32 i = 0; i < graphicsQueueCount; i++) {
    queue_t queue{};
    queue.m_type = QUEUE_FAMILY_GRAPHICS;
    vkGetDeviceQueue(device.m_handle, device.m_gfxQueueFamilyIndex.value(), i,
                     &queue.m_handle);
    m_queues.insert(queue);
  }
  for (u32 i = 0; i < transferQueueCount; i++) {
    queue_t queue{};
    queue.m_type = QUEUE_FAMILY_TRANSFER;
    vkGetDeviceQueue(device.m_handle, device.m_transferQueueFamilyIndex.value(),
                     i, &queue.m_handle);
    m_queues.insert(queue);
  }
  for (u32 i = 0; i < computeQueueCount; i++) {
    queue_t queue{};
    queue.m_type = QUEUE_FAMILY_COMPUTE;
    vkGetDeviceQueue(device.m_handle, device.m_computeQueueFamilyIndex.value(),
                     i, &queue.m_handle);
    m_queues.insert(queue);
  }

  vkGetPhysicalDeviceMemoryProperties(device.m_physicalDevice,
                                      &device.m_memoryProperties);
  return device;
}

VRendererBackend::surface_t VRendererBackend::createSurface() {
  // create surface and swapchain.
  surface_t surface{};
  if (mp_window->getBackendAPI() == GLFW_WINDOW_BACKEND) {
    const glfw::GlfwWindowBackend &backend =
        static_cast<const glfw::GlfwWindowBackend &>(mp_window->getBackend());
    VkResult result = glfwCreateWindowSurface(
        m_instance.m_handle, const_cast<GLFWwindow *>(backend.pointer()),
        nullptr, &surface.m_handle);
    ASSERT_VKRESULT(result);
  } else
    throw std::runtime_error("unsupported window backend");
  return surface;
}

void VRendererBackend::beginFrame() {
  //
  m_rootPass->beginFrame();
}

void VRendererBackend::renderFrame() {
  //
  m_rootPass->execute();
}

void VRendererBackend::endFrame() {
  //
  m_rootPass->endFrame();
}

imageview VRendererBackend::createImageView(VkImage image, const u32 width,
                                            const u32 height, VkFormat format) {
  imageview_t imageview;
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
  vkCreateImageView(m_device.m_handle, &imageViewCreateInfo, nullptr,
                    &imageview.m_handle);
  imageview.m_width = width;
  imageview.m_height = height;
  struct imageview handle(m_imageViews.insert(imageview));
  m_imageViews[handle.m_index].m_index = handle.m_index;
  return handle;
}

void VRendererBackend::destroyImageView(imageview imageViewHandle) {
  destroyImageView(getImageViewByHandle(imageViewHandle));
}

const std::pair<u32, u32> VRendererBackend::getImageViewDimensions(
    const imageview imageViewHandle) {
  m_imageViews.assertValidIndex(imageViewHandle.m_index);
  return std::make_pair(m_imageViews[imageViewHandle.m_index].m_width,
                        m_imageViews[imageViewHandle.m_index].m_height);
}

shader_module VRendererBackend::createShaderModule(const std::string path,
                                                   ShaderType shaderType) {
  //
  if (m_loadedShaders.contains(path)) {
    const shader_module handle = m_loadedShaders[path];
    m_shaders[handle.m_index].m_refCount++;
    return handle;
  }

  shader_module_t shaderModule{};
  shaderModule.m_refCount = 1;
  shaderModule.m_layout = std::nullopt;
  shaderModule.m_type = shaderType;

  const std::vector<char> spvSourceCode = sge::fileio::read(path + ".spv");
  if (shaderType == SHADER_TYPE_VERTEX) {
    const std::vector<char> glslSourceCode = sge::fileio::read(path);
    vertex_shader_input_layout_t inputLayout{};
    // read the actual glsl source code to parse vertex input layout if this is
    // a vertex shader
    std::vector<std::string> lines;
    u32 i = 0;
    u32 j = 0;
    while (i < glslSourceCode.size()) {
      std::string line = "";
      bool start = true;
      while (glslSourceCode[i] != '\n' && i < glslSourceCode.size()) {
        line.push_back(glslSourceCode[i]);
        i++;
      }
      lines.push_back(line);
      j++;
      i++;
    }

    for (u32 i = 0; i < lines.size(); i++) {
      if (std::regex_match(
              lines[i],
              std::regex(
                  ".*layout\\([a-zA-Z0-9, \t\r\n\f=]*location[ \t\r\n\f]*=[ "
                  "\t\r\n\f]*[0-9]+[a-zA-Z0-9, \t\r\n\f=]*\\)[ \t\r\n\f]+in[ "
                  "\t\r\n\f]+(vec4|vec3|vec2|float).*"))) {
        println(lines[i]);
        u32 s = lines[i].find("location") + std::string("location").size();
        while (!(lines[i][s] >= '0' && lines[i][s] <= '9')) {
          s++;
        }
        println(lines[i].substr(s));
        u32 e = s;
        while ((lines[i][e] >= '0' && lines[i][e] <= '9')) {
          e++;
        }
        const u32 location = std::stoi(lines[i].substr(s, e - s));
        std::string rest = lines[i].substr(e);
        e = rest.find(';');
        rest = rest.substr(0, e);
        GlslType type;
        if (std::regex_search(rest, std::regex("vec4"))) {
          type = GLSL_TYPE_VEC4;
        } else if (std::regex_search(rest, std::regex("vec3"))) {
          type = GLSL_TYPE_VEC3;
        } else if (std::regex_search(rest, std::regex("vec2"))) {
          type = GLSL_TYPE_VEC2;
        } else if (std::regex_search(rest, std::regex("flaot"))) {
          type = GLSL_TYPE_FLOAT;
        } else {
          throw std::runtime_error(
              "failed to parse vertex shader input layout");
        }
        vertex_input_descriptor_t inputDesc{};
        inputDesc.m_location = location;
        inputDesc.m_type = type;
        inputLayout.m_inputDescs.push_back(inputDesc);
      }
    }

    // calculate offsets and stride
    // sort by location.
    std::sort(inputLayout.m_inputDescs.begin(), inputLayout.m_inputDescs.end(),
              [](const vertex_input_descriptor_t &a,
                 const vertex_input_descriptor_t &b) {
                return a.m_location < b.m_location;
              });
    u32 offset = 0;
    for (vertex_input_descriptor_t &inputDesc : inputLayout.m_inputDescs) {
      inputDesc.m_offset = 0;
      u32 size = [&]() {
        switch (inputDesc.m_type) {
          case GLSL_TYPE_VEC4:
            return sizeof(float) * 4;
          case GLSL_TYPE_VEC3:
            return sizeof(float) * 3;
          case GLSL_TYPE_VEC2:
            return sizeof(float) * 2;
          case GLSL_TYPE_FLOAT:
            return sizeof(float);
          default:
            throw std::runtime_error("HOW DID WE GET HERE?");
        }
      }();
      offset += size;
    }
    inputLayout.m_stride = offset;
    shaderModule.m_layout = inputLayout;
  }

  // CREATE THE ACTUAL SHADER.
  VkShaderModuleCreateInfo shaderCreateInfo;
  shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shaderCreateInfo.pNext = nullptr;
  shaderCreateInfo.flags = 0;
  shaderCreateInfo.codeSize = spvSourceCode.size();
  shaderCreateInfo.pCode =
      reinterpret_cast<const uint32_t *>(spvSourceCode.data());
  VkResult result = vkCreateShaderModule(m_device.m_handle, &shaderCreateInfo,
                                         nullptr, &shaderModule.m_handle);
  ASSERT_VKRESULT(result);
  // select id for new shader.
  struct shader_module handle(m_shaders.insert(shaderModule));
  m_shaders[handle.m_index].m_index = handle.m_index;

  return handle;
}

void VRendererBackend::destroyShaderModule(shader_module shaderModuleHandle) {
  destroyShaderModule(getShaderByHandle(shaderModuleHandle));
}

void VRendererBackend::destroyShaderModule(shader_module_t &shaderModule) {
  //
  shaderModule.m_refCount--;
  if (shaderModule.m_refCount == 0) {
    m_shaders.removeAt(shaderModule.m_index);
    vkDestroyShaderModule(m_device.m_handle, shaderModule.m_handle, nullptr);
    shaderModule.m_handle = VK_NULL_HANDLE;
  }
}

pipeline_layout VRendererBackend::createPipelineLayout() {
  //
  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
  pipelineLayoutCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutCreateInfo.pNext = nullptr;
  pipelineLayoutCreateInfo.flags = 0;
  pipelineLayoutCreateInfo.setLayoutCount = 0;
  pipelineLayoutCreateInfo.pSetLayouts = nullptr;
  pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
  pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

  pipeline_layout_t pipelineLayout{};
  const VkResult result =
      vkCreatePipelineLayout(m_device.m_handle, &pipelineLayoutCreateInfo,
                             nullptr, &pipelineLayout.m_handle);
  ASSERT_VKRESULT(result);
  struct pipeline_layout handle(m_pipelineLayouts.insert(pipelineLayout));
  m_pipelineLayouts[handle.m_index].m_index = handle.m_index;
  return handle;
}
void VRendererBackend::destroyPipelineLayout(
    pipeline_layout pipelineLayoutHandle) {
  //
  destroyPipelineLayout(getPipelineLayoutByHandle(pipelineLayoutHandle));
}

void VRendererBackend::bindPipeline(pipeline pipelineHandle,
                                    command_buffer commandBufferHandle) {
  bindPipeline(getPipelineByHandle(pipelineHandle),
               getCommandBufferByHandle(commandBufferHandle));
}

u32 VRendererBackend::findSuitableMemoryType(u32 memoryTypeFilter,
                                             VkMemoryPropertyFlags properties) {
  for (uint32_t i = 0; i < m_device.m_memoryProperties.memoryTypeCount; i++) {
    if ((memoryTypeFilter & (1 << i)) &&
        (m_device.m_memoryProperties.memoryTypes[i].propertyFlags &
         properties) == properties) {
      return i;
    }
  }
  throw std::runtime_error("failed to find suitable memory type!");
}

renderpass VRendererBackend::createRenderPass(const VkFormat color_format) {
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

  renderpass_t renderpass{};
  const VkResult result = vkCreateRenderPass(
      m_device.m_handle, &renderPassCreateInfo, nullptr, &renderpass.m_handle);
  ASSERT_VKRESULT(result);
  struct renderpass handle(m_renderpasses.insert(renderpass));
  m_renderpasses[handle.m_index].m_index = handle.m_index;
  return handle;
}

void VRendererBackend::beginRenderPass(renderpass renderPassHandle,
                                       framebuffer framebufferHandle,
                                       u32 renderAreaWidth,
                                       u32 renderAreaHeight,
                                       command_buffer commandBufferHandle) {
  VkRenderPassBeginInfo renderPassBeginInfo;
  renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassBeginInfo.pNext = nullptr;
  renderPassBeginInfo.renderPass =
      getRenderPassByHandle(renderPassHandle).m_handle;
  renderPassBeginInfo.framebuffer =
      getFramebufferByHandle(framebufferHandle).m_handle;
  renderPassBeginInfo.renderArea.offset = {0, 0};
  renderPassBeginInfo.renderArea.extent = {renderAreaWidth, renderAreaHeight};

  VkClearValue clearValue;
  clearValue.color.float32[0] = 0;
  clearValue.color.float32[1] = 0;
  clearValue.color.float32[2] = 0;
  clearValue.color.float32[3] = 1;

  renderPassBeginInfo.clearValueCount = 1;
  renderPassBeginInfo.pClearValues = &clearValue;

  vkCmdBeginRenderPass(getCommandBufferByHandle(commandBufferHandle).m_handle,
                       &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VRendererBackend::endRenderPass(command_buffer commandBufferHandle) {
  vkCmdEndRenderPass(getCommandBufferByHandle(commandBufferHandle).m_handle);
}

void VRendererBackend::destroyRenderPass(renderpass renderpassHandle) {
  destroyRenderPass(getRenderPassByHandle(renderpassHandle));
}

pipeline VRendererBackend::createPipeline(
    renderpass renderPassHandle, pipeline_layout pipelineLayoutHandle,
    uint32_t viewportWidth, uint32_t viewportHeight,
    std::optional<shader_module> vertexShaderHandle,
    std::optional<shader_module> fragmentShaderHandle) {
  std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

  std::vector<VkVertexInputBindingDescription> bindingDescriptions;
  std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

  if (vertexShaderHandle.has_value()) {
    shader_module_t &shader = getShaderByHandle(vertexShaderHandle.value());
    assert(shader.m_type == SHADER_TYPE_VERTEX);
    VkPipelineShaderStageCreateInfo shaderStageCreateInfoVert;
    shaderStageCreateInfoVert.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageCreateInfoVert.pNext = nullptr;
    shaderStageCreateInfoVert.flags = 0;
    shaderStageCreateInfoVert.stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStageCreateInfoVert.module = shader.m_handle;
    shaderStageCreateInfoVert.pName = "main";
    shaderStageCreateInfoVert.pSpecializationInfo = nullptr;
    shaderStages.push_back(shaderStageCreateInfoVert);

    VkVertexInputBindingDescription bindingDesc{};
    bindingDesc.binding = 0;
    bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    assert(shader.m_layout.has_value());
    bindingDesc.stride = shader.m_layout.value().m_stride;
    bindingDescriptions.push_back(bindingDesc);

    for (vertex_input_descriptor_t &inputDesc : shader.m_layout->m_inputDescs) {
      VkVertexInputAttributeDescription attribDesc{};
      attribDesc.binding = 0;
      attribDesc.location = inputDesc.m_location;
      attribDesc.offset = inputDesc.m_offset;
      attribDesc.format = inputDesc.m_format;

      attributeDescriptions.push_back(attribDesc);
    }
  }
  if (fragmentShaderHandle.has_value()) {
    VkPipelineShaderStageCreateInfo shaderStageCreateInfoFrag;
    shaderStageCreateInfoFrag.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageCreateInfoFrag.pNext = nullptr;
    shaderStageCreateInfoFrag.flags = 0;
    shaderStageCreateInfoFrag.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStageCreateInfoFrag.module =
        getShaderByHandle(fragmentShaderHandle.value()).m_handle;
    shaderStageCreateInfoFrag.pName = "main";
    shaderStageCreateInfoFrag.pSpecializationInfo = nullptr;
    shaderStages.push_back(shaderStageCreateInfoFrag);
  }

  VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo;
  vertexInputCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputCreateInfo.pNext = nullptr;
  vertexInputCreateInfo.flags = 0;
  vertexInputCreateInfo.vertexBindingDescriptionCount =
      bindingDescriptions.size();
  vertexInputCreateInfo.pVertexBindingDescriptions = bindingDescriptions.data();
  vertexInputCreateInfo.vertexAttributeDescriptionCount =
      attributeDescriptions.size();
  vertexInputCreateInfo.pVertexAttributeDescriptions =
      attributeDescriptions.data();

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
  pipelineCreateInfo.layout =
      getPipelineLayoutByHandle(pipelineLayoutHandle).m_handle;
  pipelineCreateInfo.renderPass =
      getRenderPassByHandle(renderPassHandle).m_handle;
  pipelineCreateInfo.subpass = 0;
  pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineCreateInfo.basePipelineIndex = -1;

  pipeline_t pipeline{};
  VkResult result = vkCreateGraphicsPipelines(m_device.m_handle, VK_NULL_HANDLE,
                                              1, &pipelineCreateInfo, nullptr,
                                              &pipeline.m_handle);
  ASSERT_VKRESULT(result);
  //
  struct pipeline handle(m_pipelines.insert(pipeline));
  m_pipelines[handle.m_index].m_index = handle.m_index;
  return handle;
}

void VRendererBackend::destroyPipeline(pipeline pipelineHandle) {
  destroyPipeline(getPipelineByHandle(pipelineHandle));
}

framebuffer VRendererBackend::createFramebuffer(renderpass renderPassHandle,
                                                imageview imageViewHandle,
                                                u32 width, u32 height) {
  VkFramebufferCreateInfo framebufferCreateInfo;
  framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  framebufferCreateInfo.pNext = nullptr;
  framebufferCreateInfo.flags = 0;
  framebufferCreateInfo.renderPass =
      getRenderPassByHandle(renderPassHandle).m_handle;
  framebufferCreateInfo.attachmentCount = 1;
  framebufferCreateInfo.pAttachments =
      &getImageViewByHandle(imageViewHandle).m_handle;
  framebufferCreateInfo.width = width;
  framebufferCreateInfo.height = height;
  framebufferCreateInfo.layers = 1;
  framebuffer_t framebuffer{
      .m_imageview = imageViewHandle, .m_width = width, .m_height = height};
  const VkResult result =
      vkCreateFramebuffer(m_device.m_handle, &framebufferCreateInfo, nullptr,
                          &framebuffer.m_handle);
  ASSERT_VKRESULT(result);
  struct framebuffer handle(m_framebuffers.insert(framebuffer));
  m_framebuffers[handle.m_index].m_index = handle.m_index;
  return handle;
}

void VRendererBackend::destroyFramebuffer(framebuffer framebufferHandle) {
  destroyFramebuffer(getFramebufferByHandle(framebufferHandle));
}

const std::pair<u32, u32> VRendererBackend::getFramebufferDimensions(
    const framebuffer framebufferHandle) {
  framebuffer_t &framebuffer = getFramebufferByHandle(framebufferHandle);
  return std::make_pair(framebuffer.m_width, framebuffer.m_height);
}

command_pool VRendererBackend::createCommandPool(QueueFamilyType queueFamily) {
  u32 queueFamilyIndex = [&]() -> u32 {
    switch (queueFamily) {
      case QUEUE_FAMILY_GRAPHICS:
        assert(m_device.m_gfxQueueFamilyIndex.has_value());
        return m_device.m_gfxQueueFamilyIndex.value();
      case QUEUE_FAMILY_TRANSFER:
        assert(m_device.m_transferQueueFamilyIndex.has_value());
        return m_device.m_transferQueueFamilyIndex.value();
      case QUEUE_FAMILY_COMPUTE:
        assert(m_device.m_computeQueueFamilyIndex.has_value());
        return m_device.m_computeQueueFamilyIndex.value();
    }
    throw std::runtime_error("how did we get here!");
  }();

  VkCommandPoolCreateInfo commandPoolCreateInfo;
  commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  commandPoolCreateInfo.pNext = nullptr;
  commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
  command_pool_t pool{};
  const VkResult result = vkCreateCommandPool(
      m_device.m_handle, &commandPoolCreateInfo, nullptr, &pool.m_handle);
  ASSERT_VKRESULT(result);
  struct command_pool handle(m_commandPools.insert(pool));
  m_commandPools[handle.m_index].m_index = handle.m_index;
  return handle;
}
void VRendererBackend::resetCommandPool(command_pool commandPoolHandle) {
  const VkResult result = vkResetCommandPool(
      m_device.m_handle, getCommandPoolByHandle(commandPoolHandle).m_handle,
      VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
  ASSERT_VKRESULT(result);

  // make all ids from the buffers avaiable for reallocation.
  for (command_buffer_t &commandBuffer : m_commandBuffers) {
    if (commandBuffer.m_origin_pool.m_index == commandPoolHandle.m_index) {
      m_commandBuffers.removeAt(commandBuffer.m_index);
    }
  }
}

void VRendererBackend::destroyCommandPool(command_pool commandPoolHandle) {
  destroyCommandPool(getCommandPoolByHandle(commandPoolHandle));
}

const std::vector<command_buffer> VRendererBackend::allocateCommandBuffers(
    command_pool commandPoolHandle, u32 count) {
  VkCommandBufferAllocateInfo commandBufferAllocateInfo;
  commandBufferAllocateInfo.sType =
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  commandBufferAllocateInfo.pNext = nullptr;
  commandBufferAllocateInfo.commandPool =
      getCommandPoolByHandle(commandPoolHandle).m_handle;
  commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  commandBufferAllocateInfo.commandBufferCount = count;

  std::vector<VkCommandBuffer> vkbuffers(count);
  const VkResult result = vkAllocateCommandBuffers(
      m_device.m_handle, &commandBufferAllocateInfo, vkbuffers.data());
  ASSERT_VKRESULT(result);
  std::vector<command_buffer> handles;
  for (u32 i = 0; i < count; i++) {
    command_buffer_t buffer{.m_origin_pool = commandPoolHandle};
    buffer.m_handle = vkbuffers[i];
    buffer.m_index = m_commandBuffers.insert(buffer);
    handles.push_back(command_buffer(buffer.m_index));
  }
  return handles;
}

void VRendererBackend::freeCommandBuffers(
    const std::vector<command_buffer> &commandBufferHandles) {
  if (commandBufferHandles.empty()) return;

  std::vector<VkCommandBuffer> buffers(commandBufferHandles.size());
  std::optional<command_pool> poolHandle;
  for (const command_buffer &handle : commandBufferHandles) {
    if (!poolHandle.has_value()) {
      poolHandle = getCommandBufferByHandle(handle).m_origin_pool;
    } else {
      assert(poolHandle.value() ==
             getCommandBufferByHandle(handle).m_origin_pool);
    }
    buffers.push_back(getCommandBufferByHandle(handle).m_handle);
    m_commandBuffers.removeAt(handle.m_index);
  }
  vkFreeCommandBuffers(m_device.m_handle,
                       getCommandPoolByHandle(poolHandle.value()).m_handle,
                       buffers.size(), buffers.data());
}

void VRendererBackend::resetCommandBuffer(
    const command_buffer commandBufferHandle, const boolean releaseResources) {
  const VkResult result = vkResetCommandBuffer(
      getCommandBufferByHandle(commandBufferHandle).m_handle,
      (releaseResources ? VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT : 0));
  ASSERT_VKRESULT(result);
}

void VRendererBackend::beginCommandBuffer(command_buffer commandBufferHandle) {
  VkCommandBufferBeginInfo commandBufferBeginInfo;
  commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  commandBufferBeginInfo.pNext = nullptr;
  commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  commandBufferBeginInfo.pInheritanceInfo = nullptr;

  const VkResult result = vkBeginCommandBuffer(
      getCommandBufferByHandle(commandBufferHandle).m_handle,
      &commandBufferBeginInfo);
  ASSERT_VKRESULT(result);
}

void VRendererBackend::endCommandBuffer(command_buffer commandBufferHandle) {
  const VkResult result = vkEndCommandBuffer(
      getCommandBufferByHandle(commandBufferHandle).m_handle);
  ASSERT_VKRESULT(result);
}

semaphore VRendererBackend::createSemaphore() {
  VkSemaphoreCreateInfo semaphoreCreateInfo;
  semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  semaphoreCreateInfo.pNext = nullptr;
  semaphoreCreateInfo.flags = 0;
  semaphore_t semaphore{};
  VkResult result = vkCreateSemaphore(m_device.m_handle, &semaphoreCreateInfo,
                                      nullptr, &semaphore.m_handle);
  ASSERT_VKRESULT(result);
  struct semaphore handle(m_semaphores.insert(semaphore));
  m_semaphores[handle.m_index].m_index = handle.m_index;
  return handle;
}

void VRendererBackend::destroySemaphore(semaphore semaphoreHandle) {
  destroySemaphore(getSemaphoreByHandle(semaphoreHandle));
}

void VRendererBackend::drawCall(uint32_t vertexCount, uint32_t instanceCount,
                                command_buffer commandBufferHandle) {
  vkCmdDraw(getCommandBufferByHandle(commandBufferHandle).m_handle, vertexCount,
            instanceCount, 0, 0);
}
boolean VRendererBackend::acquireNextSwapchainFrame(semaphore signalSem) {
  VkResult result =
      vkAcquireNextImageKHR(m_device.m_handle, m_swapchain.m_handle,
                            std::numeric_limits<uint64_t>::max(),
                            getSemaphoreByHandle(signalSem).m_handle,
                            VK_NULL_HANDLE, &m_swapchain.m_frameIndex);
  if (result == VK_ERROR_OUT_OF_DATE_KHR) return true;
  if (result == VK_SUBOPTIMAL_KHR) return false;
  ASSERT_VKRESULT(result);
  return false;
}

imageview VRendererBackend::getCurrentSwapchainImageView() {
  return m_swapchain.m_imageViewsHandles[m_swapchain.m_frameIndex];
}

u32 VRendererBackend::getCurrentSwapchainFrameIndex() {
  return m_swapchain.m_frameIndex;
}

boolean VRendererBackend::presentQueue(
    queue queueHandle, const std::vector<semaphore> &waitSemaphoreHandles) {
  VkPresentInfoKHR presentInfo;
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.pNext = nullptr;
  presentInfo.waitSemaphoreCount = waitSemaphoreHandles.size();
  std::vector<VkSemaphore> waitSemaphores(waitSemaphoreHandles.size());
  for (u32 i = 0; i < waitSemaphoreHandles.size(); i++) {
    waitSemaphores[i] = getSemaphoreByHandle(waitSemaphoreHandles[i]).m_handle;
  }
  presentInfo.pWaitSemaphores = waitSemaphores.data();
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &m_swapchain.m_handle;
  presentInfo.pImageIndices = &m_swapchain.m_frameIndex;
  presentInfo.pResults = nullptr;
  VkResult result =
      vkQueuePresentKHR(getQueueByHandle(queueHandle).m_handle, &presentInfo);
  if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR) {
    return true;
  }
  ASSERT_VKRESULT(result);
  return false;
}

void VRendererBackend::submitCommandBuffers(
    queue queueHandle, const std::vector<command_buffer> &commandBufferHandles,
    const std::vector<semaphore> &waitSemaphoreHandles,
    const std::vector<semaphore> &signalSemaphoreHandles,
    const std::optional<fence> fenceHandle) {
  std::vector<VkSemaphore> waitSemaphores(waitSemaphoreHandles.size());
  for (u32 i = 0; i < waitSemaphoreHandles.size(); i++) {
    waitSemaphores[i] = getSemaphoreByHandle(waitSemaphoreHandles[i]).m_handle;
  }

  std::vector<VkCommandBuffer> buffers(commandBufferHandles.size());
  for (u32 i = 0; i < commandBufferHandles.size(); i++) {
    buffers[i] = getCommandBufferByHandle(commandBufferHandles[i]).m_handle;
  }

  std::vector<VkSemaphore> signalSemaphore(signalSemaphoreHandles.size());
  for (u32 i = 0; i < signalSemaphoreHandles.size(); i++) {
    signalSemaphore[i] =
        getSemaphoreByHandle(signalSemaphoreHandles[i]).m_handle;
  }

  VkSubmitInfo submitInfo;
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.pNext = nullptr;
  submitInfo.waitSemaphoreCount = waitSemaphores.size();
  submitInfo.pWaitSemaphores = waitSemaphores.data();
  VkPipelineStageFlags waitStageMasks = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.pWaitDstStageMask = &waitStageMasks;
  submitInfo.commandBufferCount = buffers.size();
  submitInfo.pCommandBuffers = buffers.data();
  submitInfo.signalSemaphoreCount = signalSemaphore.size();
  submitInfo.pSignalSemaphores = signalSemaphore.data();

  VkFence vkFence = VK_NULL_HANDLE;
  if (fenceHandle.has_value()) {
    vkFence = getFenceByHandle(fenceHandle.value()).m_handle;
  }

  const VkResult result = vkQueueSubmit(getQueueByHandle(queueHandle).m_handle,
                                        1, &submitInfo, vkFence);
  ASSERT_VKRESULT(result);
}

fence VRendererBackend::createFence() {
  VkFenceCreateInfo createInfo;
  createInfo.pNext = nullptr;
  createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  createInfo.flags = 0;
  fence_t fence{};
  VkResult result =
      vkCreateFence(m_device.m_handle, &createInfo, nullptr, &fence.m_handle);
  ASSERT_VKRESULT(result);
  struct fence handle(m_fences.insert(fence));
  m_fences[handle.m_index].m_index = handle.m_index;
  return handle;
}

void VRendererBackend::destroyFence(const fence fenceHandle) {
  destroyFence(getFenceByHandle(fenceHandle));
}

void VRendererBackend::resetFence(const fence fenceHandle) {
  const VkResult result = vkResetFences(
      m_device.m_handle, 1, &getFenceByHandle(fenceHandle).m_handle);
  ASSERT_VKRESULT(result);
}

void VRendererBackend::waitForFence(const fence fenceHandle) {
  const VkResult result = vkWaitForFences(
      m_device.m_handle, 1, &getFenceByHandle(fenceHandle).m_handle, true,
      std::numeric_limits<u64>::max());
  ASSERT_VKRESULT(result);
}

void VRendererBackend::waitDeviceIdle() { vkDeviceWaitIdle(m_device.m_handle); }

vertex_buffer VRendererBackend::createVertexBuffer(const u32 byteSize,
                                                   boolean exlusiveSharing) {
  if (!exlusiveSharing)
    throw std::runtime_error(
        "concurrent sharing mode is not yet implemented for vertex buffers");

  VkBufferCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  createInfo.pNext = nullptr;
  createInfo.flags = 0;
  createInfo.size = byteSize;
  createInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  createInfo.sharingMode = (exlusiveSharing ? VK_SHARING_MODE_EXCLUSIVE
                                            : VK_SHARING_MODE_CONCURRENT);
  createInfo.queueFamilyIndexCount = 0;
  createInfo.pQueueFamilyIndices = nullptr;
  vertex_buffer_t vertexBuffer;
  VkResult result = vkCreateBuffer(m_device.m_handle, &createInfo, nullptr,
                                   &vertexBuffer.m_handle);
  ASSERT_VKRESULT(result);

  VkMemoryRequirements memReq;
  vkGetBufferMemoryRequirements(m_device.m_handle, vertexBuffer.m_handle,
                                &memReq);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.pNext = nullptr;
  allocInfo.allocationSize = memReq.size;
  allocInfo.memoryTypeIndex = findSuitableMemoryType(
      memReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  result = vkAllocateMemory(m_device.m_handle, &allocInfo, nullptr,
                            &vertexBuffer.m_memory);
  ASSERT_VKRESULT(result);
  vertexBuffer.m_size = createInfo.size;

  struct vertex_buffer handle(m_vertexBuffers.insert(vertexBuffer));
  m_vertexBuffers[handle.m_index].m_index = handle.m_index;
  return handle;
}

void VRendererBackend::destroyVertexBuffer(vertex_buffer vertexBuffer) {
  destroyVertexBuffer(getVertexBufferByHandle(vertexBuffer));
}

void VRendererBackend::destroyVertexBuffer(vertex_buffer_t vertexBuffer) {
  m_vertexBuffers.removeAt(vertexBuffer.m_index);
  vkDestroyBuffer(m_device.m_handle, vertexBuffer.m_handle, nullptr);
  vkFreeMemory(m_device.m_handle, vertexBuffer.m_memory, nullptr);
}

void VRendererBackend::uploadToVertexBuffer(vertex_buffer vertexBuffer,
                                            void *data, u32 offset,
                                            std::optional<u32> size) {
  if (size.has_value()) {
    uploadToVertexBuffer(getVertexBufferByHandle(vertexBuffer), data, offset,
                         size.value());
  } else {
    vertex_buffer_t &buffer = getVertexBufferByHandle(vertexBuffer);
    uploadToVertexBuffer(buffer, data, offset, buffer.m_size);
  }
}
void VRendererBackend::uploadToVertexBuffer(vertex_buffer_t &vertexBuffer,
                                            void *data, u32 offset, u32 size) {
  //
  vkBindBufferMemory(m_device.m_handle, vertexBuffer.m_handle,
                     vertexBuffer.m_memory, 0);
  void *memory;
  VkResult result = vkMapMemory(m_device.m_handle, vertexBuffer.m_memory,
                                offset, size, 0, &memory);
  ASSERT_VKRESULT(result);
  std::memcpy(memory, data, size);
  vkUnmapMemory(m_device.m_handle, vertexBuffer.m_memory);
}
void VRendererBackend::bindVertexBuffer(vertex_buffer vertexBufferHandle,
                                        command_buffer commandBufferHandle) {
  VkBuffer vertexBuffers[] = {
      getVertexBufferByHandle(vertexBufferHandle).m_handle};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(getCommandBufferByHandle(commandBufferHandle).m_handle,
                         0, 1, vertexBuffers, offsets);
}

queue VRendererBackend::getAnyGraphicsQueue() {
  for (queue_t &queue : m_queues) {
    if (queue.m_type == QUEUE_FAMILY_GRAPHICS) {
      struct queue handle(queue.m_index);
      return handle;
    }
  }
  throw std::runtime_error("no graphics queue avaiable");
}
queue VRendererBackend::getAnyTransferQueue() {
  for (queue_t &queue : m_queues) {
    if (queue.m_type == QUEUE_FAMILY_TRANSFER) {
      struct queue handle(queue.m_index);
      return handle;
    }
  }
  throw std::runtime_error("no transfer queue avaiable");
}
queue VRendererBackend::getAnyComputeQueue() {
  for (queue_t &queue : m_queues) {
    if (queue.m_type == QUEUE_FAMILY_COMPUTE) {
      struct queue handle(queue.m_index);
      return handle;
    }
  }
  throw std::runtime_error("no compute queue avaiable");
}

void VRendererBackend::updateSwapchainSupport() {
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      m_device.m_physicalDevice, m_surface.m_handle, &m_surface.m_capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(
      m_device.m_physicalDevice, m_surface.m_handle, &formatCount, nullptr);

  if (formatCount != 0) {
    m_surface.m_formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_device.m_physicalDevice,
                                         m_surface.m_handle, &formatCount,
                                         m_surface.m_formats.data());
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(m_device.m_physicalDevice,
                                            m_surface.m_handle,
                                            &presentModeCount, nullptr);

  if (presentModeCount != 0) {
    m_surface.m_presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        m_device.m_physicalDevice, m_surface.m_handle, &presentModeCount,
        m_surface.m_presentModes.data());
  }

  bool supportsFormat = false;
  for (VkSurfaceFormatKHR format : m_surface.m_formats) {
    if (format.format == SURFACE_COLOR_FORMAT) {
      supportsFormat = true;
    }
  }
  assert(supportsFormat);

  VkBool32 surfaceSupport = false;
  const VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(
      m_device.m_physicalDevice, 0, m_surface.m_handle, &surfaceSupport);
  ASSERT_VKRESULT(result);
  assert(surfaceSupport);
}

void VRendererBackend::createSwapchain() {
  updateSwapchainSupport();

  VkExtent2D extend;

  if (m_surface.m_capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    extend = m_surface.m_capabilities.currentExtent;
  } else {
    int width = mp_window->getWidth(), height = mp_window->getHeight();
    VkExtent2D actualExtent = {static_cast<uint32_t>(width),
                               static_cast<uint32_t>(height)};

    actualExtent.width = std::clamp(
        actualExtent.width, m_surface.m_capabilities.minImageExtent.width,
        m_surface.m_capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(
        actualExtent.height, m_surface.m_capabilities.minImageExtent.height,
        m_surface.m_capabilities.maxImageExtent.height);

    extend = actualExtent;
  }

  VkSwapchainCreateInfoKHR swapchainCreateInfo;
  swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchainCreateInfo.pNext = nullptr;
  swapchainCreateInfo.flags = 0;
  swapchainCreateInfo.surface = m_surface.m_handle;
  swapchainCreateInfo.minImageCount = std::min(
      m_surface.m_capabilities.minImageCount, (uint32_t)MAX_SWAPCHAIN_IMAGES);
  swapchainCreateInfo.imageFormat =
      VRendererBackend::SURFACE_COLOR_FORMAT;  // TODO select dynamicly
  swapchainCreateInfo.imageColorSpace =
      VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;  // TODO check if valid.
  swapchainCreateInfo.imageExtent = extend;
  swapchainCreateInfo.imageArrayLayers = 1;
  swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  swapchainCreateInfo.queueFamilyIndexCount = 0;
  swapchainCreateInfo.pQueueFamilyIndices = nullptr;
  swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchainCreateInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
  swapchainCreateInfo.clipped = VK_TRUE;
  swapchainCreateInfo.oldSwapchain = m_swapchain.m_handle;

  VkResult result = vkCreateSwapchainKHR(
      m_device.m_handle, &swapchainCreateInfo, nullptr, &m_swapchain.m_handle);
  ASSERT_VKRESULT(result);
  uint32_t n_swapchainImages;
  result = vkGetSwapchainImagesKHR(m_device.m_handle, m_swapchain.m_handle,
                                   &n_swapchainImages, nullptr);
  ASSERT_VKRESULT(result);
  std::vector<VkImage> swapchainImages(n_swapchainImages);
  result = vkGetSwapchainImagesKHR(m_device.m_handle, m_swapchain.m_handle,
                                   &n_swapchainImages, swapchainImages.data());
  ASSERT_VKRESULT(result);

  m_swapchain.m_imageViewsHandles.resize(n_swapchainImages, imageview());
  for (unsigned int i = 0; i < n_swapchainImages; i++) {
    imageview imageviewHandle =
        createImageView(swapchainImages[i], extend.width, extend.height,
                        VRendererBackend::SURFACE_COLOR_FORMAT);
    m_swapchain.m_imageViewsHandles[i] = imageviewHandle;
  }
}

VRendererBackend::imageview_t &VRendererBackend::getImageViewByHandle(
    const imageview imageViewHandle) {
  assert(imageViewHandle.m_index != INVALID_INDEX_HANDLE);
  return m_imageViews.at(imageViewHandle.m_index);
}

VRendererBackend::pipeline_layout_t &
VRendererBackend::getPipelineLayoutByHandle(
    const pipeline_layout pipelineLayoutHandle) {
  assert(pipelineLayoutHandle.m_index != INVALID_INDEX_HANDLE);
  return m_pipelineLayouts.at(pipelineLayoutHandle.m_index);
}
VRendererBackend::pipeline_t &VRendererBackend::getPipelineByHandle(
    const pipeline pipelineHandle) {
  assert(pipelineHandle.m_index != INVALID_INDEX_HANDLE);
  return m_pipelines.at(pipelineHandle.m_index);
}
VRendererBackend::renderpass_t &VRendererBackend::getRenderPassByHandle(
    const renderpass renderPassHandle) {
  assert(renderPassHandle.m_index != INVALID_INDEX_HANDLE);
  return m_renderpasses.at(renderPassHandle.m_index);
}
VRendererBackend::shader_module_t &VRendererBackend::getShaderByHandle(
    const shader_module shaderHandle) {
  assert(shaderHandle.m_index != INVALID_INDEX_HANDLE);
  return m_shaders.at(shaderHandle.m_index);
}

VRendererBackend::framebuffer_t &VRendererBackend::getFramebufferByHandle(
    const framebuffer framebufferHandle) {
  assert(framebufferHandle.m_index != INVALID_INDEX_HANDLE);
  return m_framebuffers.at(framebufferHandle.m_index);
}

VRendererBackend::command_pool_t &VRendererBackend::getCommandPoolByHandle(
    const command_pool commandPoolHandle) {
  assert(commandPoolHandle.m_index != INVALID_INDEX_HANDLE);
  return m_commandPools.at(commandPoolHandle.m_index);
}

VRendererBackend::command_buffer_t &VRendererBackend::getCommandBufferByHandle(
    const command_buffer commandBufferHandle) {
  assert(commandBufferHandle.m_index != INVALID_INDEX_HANDLE);
  return m_commandBuffers.at(commandBufferHandle.m_index);
}

VRendererBackend::semaphore_t &VRendererBackend::getSemaphoreByHandle(
    const semaphore semaphoreHandle) {
  assert(semaphoreHandle.m_index != INVALID_INDEX_HANDLE);
  return m_semaphores.at(semaphoreHandle.m_index);
}

VRendererBackend::queue_t &VRendererBackend::getQueueByHandle(
    const queue queueHandle) {
  assert(queueHandle.m_index != INVALID_INDEX_HANDLE);
  return m_queues.at(queueHandle.m_index);
}

VRendererBackend::fence_t &VRendererBackend::getFenceByHandle(
    const fence fenceHandle) {
  assert(fenceHandle.m_index != INVALID_INDEX_HANDLE);
  return m_fences.at(fenceHandle.m_index);
}

VRendererBackend::vertex_buffer_t &VRendererBackend::getVertexBufferByHandle(
    const vertex_buffer vertexBufferHandle) {
  assert(vertexBufferHandle.m_index != INVALID_INDEX_HANDLE);
  return m_vertexBuffers.at(vertexBufferHandle.m_index);
}

void VRendererBackend::destroyCommandPool(command_pool_t &commandPool) {
  //
  m_commandPools.removeAt(commandPool.m_index);
  vkDestroyCommandPool(m_device.m_handle, commandPool.m_handle, nullptr);
  commandPool.m_handle = VK_NULL_HANDLE;
}
void VRendererBackend::destroySemaphore(semaphore_t &semaphore) {
  //
  m_semaphores.removeAt(semaphore.m_index);
  vkDestroySemaphore(m_device.m_handle, semaphore.m_handle, nullptr);
  semaphore.m_handle = VK_NULL_HANDLE;
}
void VRendererBackend::destroyPipeline(pipeline_t &pipeline) {
  //
  vkDestroyPipeline(m_device.m_handle, pipeline.m_handle, nullptr);
  m_pipelines.removeAt(pipeline.m_index);
  pipeline.m_handle = VK_NULL_HANDLE;
}
void VRendererBackend::destroyFramebuffer(framebuffer_t &framebuffer) {
  m_framebuffers.removeAt(framebuffer.m_index);
  vkDestroyFramebuffer(m_device.m_handle, framebuffer.m_handle, nullptr);
  framebuffer.m_handle = VK_NULL_HANDLE;
  //
}
void VRendererBackend::destroyPipelineLayout(
    pipeline_layout_t &pipelineLayout) {
  //
  m_pipelineLayouts.removeAt(pipelineLayout.m_index);
  vkDestroyPipelineLayout(m_device.m_handle, pipelineLayout.m_handle, nullptr);
  pipelineLayout.m_handle = VK_NULL_HANDLE;
}
void VRendererBackend::destroyRenderPass(renderpass_t &renderpass) {
  //
  m_renderpasses.removeAt(renderpass.m_index);
  vkDestroyRenderPass(m_device.m_handle, renderpass.m_handle, nullptr);
  renderpass.m_handle = VK_NULL_HANDLE;
}
void VRendererBackend::destroyImageView(imageview_t &imageview) {
  //
  m_imageViews.removeAt(imageview.m_index);
  vkDestroyImageView(m_device.m_handle, imageview.m_handle, nullptr);
  imageview.m_handle = VK_NULL_HANDLE;
}
void VRendererBackend::destroySwapchain(swapchain_t &swapchain) {
  //
  vkDestroySwapchainKHR(m_device.m_handle, swapchain.m_handle, nullptr);
}
void VRendererBackend::destroySurface(surface_t &surface) {
  //
  vkDestroySurfaceKHR(m_instance.m_handle, m_surface.m_handle, nullptr);
}
void VRendererBackend::destroyDevice(device_t &device) {
  //
  vkDestroyDevice(m_device.m_handle, nullptr);
}
void VRendererBackend::destroyInstance(instance_t &instance) {
  //
  vkDestroyInstance(m_instance.m_handle, nullptr);
}
void VRendererBackend::destroyFence(fence_t &fence) {
  //
  m_fences.removeAt(fence.m_index);
  vkDestroyFence(m_device.m_handle, fence.m_handle, nullptr);
  fence.m_handle = VK_NULL_HANDLE;
}

void VRendererBackend::bindPipeline(pipeline_t &pipeline,
                                    command_buffer_t &commandBuffer) {
  //
  vkCmdBindPipeline(commandBuffer.m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipeline.m_handle);
}

}  // namespace sge::vulkan
