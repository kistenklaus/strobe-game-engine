#include "strobe/gpu/device/device.hpp"
#include "strobe/core/containers/small_vector.hpp"
#include "strobe/gpu/device/binary_semaphore_impl.hpp"
#include "strobe/gpu/device/buffer_impl.hpp"
#include "strobe/gpu/device/buffer_usage_utils.hpp"
#include "strobe/gpu/device/command_pool_impl.hpp"
#include "strobe/gpu/device/device_impl.hpp"
#include "strobe/gpu/device/fence.hpp"
#include "strobe/gpu/device/fence_impl.hpp"
#include "strobe/gpu/device/format_utilts.hpp"
#include "strobe/gpu/device/handle.hpp"
#include "strobe/gpu/device/image_flags_utils.hpp"
#include "strobe/gpu/device/image_impl.hpp"
#include "strobe/gpu/device/image_type_utils.hpp"
#include "strobe/gpu/device/image_usage_utils.hpp"
#include "strobe/gpu/device/memory_usage_utils.hpp"
#include "strobe/gpu/device/queue_impl.hpp"
#include "strobe/gpu/device/sample_count_utils.hpp"
#include "strobe/gpu/device/shader_object_impl.hpp"
#include "strobe/gpu/device/shader_stage_utils.hpp"
#include "strobe/gpu/device/surface_impl.hpp"
#include "strobe/gpu/device/swapchain_impl.hpp"
#include "strobe/gpu/device/timeline_semaphore_impl.hpp"
#include "strobe/gpu/vulkan/buffer.hpp"
#include "strobe/gpu/vulkan/fence.hpp"
#include "strobe/gpu/vulkan/image.hpp"
#include "strobe/gpu/vulkan/shader_object.hpp"
#include "strobe/gpu/vulkan/surface.hpp"
#include "strobe/gpu/vulkan/timeline_semaphore.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::gpu {

Device::Device(const DeviceCreateInfo &createInfo)
    : m_handle(make_void_handle<DeviceImpl>(createInfo)) {}

Device::Device(const Device &o) noexcept : m_handle(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<DeviceImpl>(m_handle);
  }
}

Device::Device(Device &&o) noexcept
    : m_handle(std::exchange(o.m_handle, nullptr)) {}

Device &Device::operator=(const Device &o) noexcept {
  if (this == &o) {
    return *this;
  }
  // Pin first in case both objects refer to the same control block.
  if (o.m_handle != nullptr) {
    pin_void_handle<DeviceImpl>(o.m_handle);
  }
  unpin_void_handle<DeviceImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

Device &Device::operator=(Device &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<DeviceImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

Device::~Device() noexcept { unpin_void_handle<DeviceImpl>(m_handle); }

Queue Device::get_queue(QueueFlags flags) {
  ZoneScopedN("Device::get_queue");
  auto *impl = void_handle_ptr<DeviceImpl>(m_handle);
  const bool exclusive = (flags & QueueFlags::exclusive) != 0;
  VkQueueFlags required = 0;
  if ((flags & QueueFlags::graphics) != 0) {
    required |= VK_QUEUE_GRAPHICS_BIT;
  }
  if ((flags & QueueFlags::compute) != 0) {
    required |= VK_QUEUE_COMPUTE_BIT;
  }
  if ((flags & QueueFlags::transfer) != 0) {
    required |= VK_QUEUE_TRANSFER_BIT;
  }
  const bool requirePresent = (flags & QueueFlags::present) != 0;
  struct Candidate {
    uint32_t index;
    uint32_t extraCapabilities;
    uint32_t users;
  };
  std::array<Candidate, NATIVE_QUEUE_COUNT> candidates;
  uint32_t candidateCount = 0;
  for (uint32_t i = 0; i < NATIVE_QUEUE_COUNT; ++i) {
    NativeQueue &native = impl->nativeQueues[i];
    const vulkan::QueueDescription &desc = NATIVE_QUEUE_DESC[i];
    if (!native.available()) {
      continue;
    }
    // The queue must provide every requested Vulkan capability.
    if ((desc.require & required) != required) {
      continue;
    }
    // Presentation is an additional capability outside VkQueueFlags.
    if (requirePresent && desc.present == vulkan::feature::disable) {
      continue;
    }
    const VkQueueFlags extra = desc.require & ~required;
    uint32_t extraCapabilities = std::popcount(extra);
    // Prefer a non-presentation queue if presentation was not requested.
    if (!requirePresent && desc.present != vulkan::feature::disable) {
      ++extraCapabilities;
    }
    candidates[candidateCount++] = Candidate{
        .index = i,
        .extraCapabilities = extraCapabilities,
        .users = native.user_count(),
    };
  }

  const auto less = [](const Candidate &a, const Candidate &b) {
    if (a.extraCapabilities != b.extraCapabilities) {
      return a.extraCapabilities < b.extraCapabilities;
    }
    return a.users < b.users;
  };

  // NATIVE_QUEUE_COUNT is tiny, so insertion sort avoids std::sort's
  // unnecessary machinery (and the GCC -Warray-bounds false positive).
  for (uint32_t i = 1; i < candidateCount; ++i) {
    Candidate candidate = candidates[i];
    uint32_t j = i;
    while (j > 0 && less(candidate, candidates[j - 1])) {
      candidates[j] = candidates[j - 1];
      --j;
    }
    candidates[j] = candidate;
  }

  // The state may have changed since ranking, so acquisition itself
  // must still be atomic. If the preferred queue is no longer available,
  // try the next-best candidate.
  for (uint32_t i = 0; i < candidateCount; ++i) {
    NativeQueue &native = impl->nativeQueues[candidates[i].index];
    if (!native.try_acquire(exclusive)) {
      continue;
    }
    return Queue{make_void_handle<QueueImpl>(*this, &native, flags)};
  }

  return {};
}

Swapchain Device::create_swapchain(GLFWwindow *window,
                                   const SwapchainCreateInfo &createInfo) {
  ZoneScopedN("Device::create_swapchain");
  auto *impl = void_handle_ptr<DeviceImpl>(m_handle);
  vulkan::Surface surface = vulkan::create_surface(impl->context.get(), window);
  Surface surface_handle{make_void_handle<SurfaceImpl>(impl->context, surface)};

  const vulkan::SurfaceCapabilities capabilities =
      vulkan::query_surface_capabilities(impl->context.get(), surface);

  uint32_t minImageCount = capabilities.minImageCount + 1;
  if (capabilities.maxImageCount != 0) {
    minImageCount = std::min(minImageCount, capabilities.maxImageCount);
  }

  auto formats = vulkan::query_surface_formats(impl->context.get(), surface);
  if (formats.empty()) {
    vulkan::destroy_surface(impl->context.get(), surface);
    throw std::runtime_error("Surface exposes no supported formats");
  }
  VkSurfaceFormatKHR format = formats[0];
  for (const VkSurfaceFormatKHR &candidate : formats) {
    if (candidate.format == VK_FORMAT_B8G8R8A8_SRGB &&
        candidate.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      format = candidate;
      break;
    }
  }
  if (!(format.format == VK_FORMAT_B8G8R8A8_SRGB &&
        format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)) {
    for (const VkSurfaceFormatKHR &candidate : formats) {
      if (candidate.format == VK_FORMAT_R8G8B8A8_SRGB &&
          candidate.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        format = candidate;
        break;
      }
    }
  }

  auto present_modes =
      vulkan::query_present_modes(impl->context.get(), surface);
  VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
  if (!createInfo.vsync) {
    if (std::find(present_modes.begin(), present_modes.end(),
                  VK_PRESENT_MODE_MAILBOX_KHR) != present_modes.end()) {
      presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
    } else if (std::find(present_modes.begin(), present_modes.end(),
                         VK_PRESENT_MODE_IMMEDIATE_KHR) !=
               present_modes.end()) {
      presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    }
  }

  const VkImageUsageFlags imageUsage = to_vk_image_usage(createInfo.imageUsage);
  if ((capabilities.supportedUsageFlags & imageUsage) != imageUsage) {
    vulkan::destroy_surface(impl->context.get(), surface);
    throw std::runtime_error(
        "Requested swapchain image usage is not supported");
  }

  SmallVector<uint32_t, 4> queueFamilyIndicies{};
  for (uint32_t i = 0; i < NATIVE_QUEUE_COUNT; ++i) {
    if (NATIVE_QUEUE_DESC[i].present == vulkan::feature::disable) {
      continue;
    }
    const auto &native = impl->nativeQueues[i];
    if (!native.queue) {
      continue;
    }
    const uint32_t familyIndex = native.queue.family;
    bool alreadyPresent = false;
    for (uint32_t existing : queueFamilyIndicies) {
      if (existing == familyIndex) {
        alreadyPresent = true;
        break;
      }
    }
    if (!alreadyPresent) {
      queueFamilyIndicies.push_back(familyIndex);
    }
  }
  if (queueFamilyIndicies.empty()) {
    vulkan::destroy_surface(impl->context.get(), surface);
    throw std::runtime_error("Device has no presentation queue family");
  }

  uvec2 extent = createInfo.extent;
  if (capabilities.currentExtent.width != UINT32_MAX) {
    extent = {
        capabilities.currentExtent.width,
        capabilities.currentExtent.height,
    };
  } else if (extent.x() != 0 && extent.y() != 0) {
    extent.x() = std::clamp(extent.x(), capabilities.minImageExtent.width,
                            capabilities.maxImageExtent.width);
    extent.y() = std::clamp(extent.y(), capabilities.minImageExtent.height,
                            capabilities.maxImageExtent.height);
  }
  return Swapchain{make_void_handle<SwapchainImpl>(
      impl->context, std::move(surface_handle), minImageCount, format,
      presentMode, imageUsage, span<uint32_t>{queueFamilyIndicies}, extent,
      createInfo.clipped)};
}

CommandPool Device::create_cmd_pool(const Queue &queue) {
  ZoneScopedN("Device::create_cmd_pool");
  auto *impl = void_handle_ptr<DeviceImpl>(m_handle);
  auto *queue_impl = void_handle_ptr<QueueImpl>(queue.m_handle);

  return CommandPool{make_void_handle<CommandPoolImpl>(
      impl->context, queue_impl->native->queue)};
}

Image Device::create_image(const ImageCreateInfo &createInfo) {
  ZoneScopedN("Device::create_image");
  auto *impl = void_handle_ptr<DeviceImpl>(m_handle);

  vulkan::ImageInfo info{
      .type = to_vk_image_type(createInfo.type),
      .format = to_vk_format(createInfo.format),
      .extent =
          VkExtent3D{
              .width = createInfo.extent.x(),
              .height = createInfo.extent.y(),
              .depth = createInfo.extent.z(),
          },
      .mip_levels = createInfo.mip_levels,
      .array_layers = createInfo.arrayLayers,
      .samples = to_vk_sample_count(createInfo.samples),
      .tiling = createInfo.linearTiling ? VK_IMAGE_TILING_LINEAR
                                        : VK_IMAGE_TILING_OPTIMAL,
      .usage = to_vk_image_usage(createInfo.usage),
      .flags = to_vk_image_flags(createInfo.flags),
      .initial_layout = VK_IMAGE_LAYOUT_UNDEFINED,
      .memory_usage = to_vulkan_memory_usage(createInfo.memoryUsage),
  };
  vulkan::Image image = vulkan::create_image(impl->context.get(), info);

  return Image{make_void_handle<ImageImpl>(
      impl->context, image, createInfo.type, createInfo.format,
      createInfo.extent, createInfo.mip_levels, createInfo.arrayLayers,
      createInfo.samples)};
}
BinarySemaphore Device::create_binary_semaphore() {
  ZoneScopedN("Device::create_binary_semaphore");
  auto *impl = void_handle_ptr<DeviceImpl>(m_handle);
  vulkan::BinarySemaphore sem =
      vulkan::create_binary_semaphore(impl->context.get());
  return BinarySemaphore{
      make_void_handle<BinarySemaphoreImpl>(impl->context, sem)};
}

TimelineSemaphore Device::create_timeline_semaphore(uint64_t initalValue) {
  ZoneScopedN("Device::create_timeline_semaphore");
  auto *impl = void_handle_ptr<DeviceImpl>(m_handle);
  vulkan::TimelineSemaphore sem = vulkan::create_timeline_semaphore(
      impl->context.get(), {.initalValue = initalValue});
  return TimelineSemaphore{
      make_void_handle<TimelineSemaphoreImpl>(impl->context, sem)};
}

Fence Device::create_fence(bool signaled) {
  ZoneScopedN("Device::create_fence");
  auto *impl = void_handle_ptr<DeviceImpl>(m_handle);
  vulkan::Fence fence = vulkan::create_fence(
      impl->context.get(),
      {.flags = signaled ? static_cast<VkFenceCreateFlags>(
                               VK_FENCE_CREATE_SIGNALED_BIT)
                         : VkFenceCreateFlags{0}});
  return Fence{make_void_handle<FenceImpl>(impl->context, fence)};
}

Buffer Device::create_buffer(const BufferCreateInfo &info) {
  ZoneScopedN("Device::create_buffer");
  auto *impl = void_handle_ptr<DeviceImpl>(m_handle);
  vulkan::Buffer buffer = vulkan::create_buffer(
      impl->context.get(),
      vulkan::BufferInfo{
          .size = info.size,
          .usage = to_vk_buffer_usage(info.usage),
          .memory_usage = to_vulkan_memory_usage(info.memory_usage),
      });
  VkDeviceAddress address =
      vulkan::get_buffer_device_address(impl->context.get(), buffer);
  return Buffer{
      make_void_handle<BufferImpl>(impl->context, buffer, info.size, address)};
}

VertexShader Device::create_vertex_shader(const VertexShaderCreateInfo &info) {
  ZoneScopedN("Device::create_vertex_shader");
  auto *impl = void_handle_ptr<DeviceImpl>(m_handle);

  SmallVector<VkPushConstantRange> pcRange{};
  for (uint32_t i = 0; i < info.pushConstantRange.size(); ++i) {
    const auto &range = info.pushConstantRange[i];
    pcRange.push_back({
        .stageFlags = to_vk_shader_stage(range.stage),
        .offset = range.offset,
        .size = range.size,
    });
  }
  vulkan::ShaderObject so = vulkan::create_shader_object(
      impl->context.get(), vulkan::ShaderObjectCreateInfo{
                               .stage = VK_SHADER_STAGE_VERTEX_BIT,
                               .flags = 0,
                               .nextStage = to_vk_shader_stage(info.nextStage),
                               .spirv = info.spirv,
                               .pushConstantRange = pcRange,
                               .specInfo = nullptr,
                           });
  return VertexShader{make_void_handle<ShaderObjectImpl>(impl->context, so)};
}

FragmentShader
Device::create_fragment_shader(const FragmentShaderCreateInfo &info) {
  ZoneScopedN("Device::create_fragment_shader");
  auto *impl = void_handle_ptr<DeviceImpl>(m_handle);

  SmallVector<VkPushConstantRange> pcRange{};
  for (uint32_t i = 0; i < info.pushConstantRange.size(); ++i) {
    const auto &range = info.pushConstantRange[i];
    pcRange.push_back({
        .stageFlags = to_vk_shader_stage(range.stage),
        .offset = range.offset,
        .size = range.size,
    });
  }
  vulkan::ShaderObject so = vulkan::create_shader_object(
      impl->context.get(), vulkan::ShaderObjectCreateInfo{
                               .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                               .flags = 0,
                               .spirv = info.spirv,
                               .pushConstantRange = pcRange,
                               .specInfo = nullptr,
                           });
  return FragmentShader{make_void_handle<ShaderObjectImpl>(impl->context, so)};
}

const DeviceInfo &Device::info() const noexcept {
  auto *impl = void_handle_ptr<DeviceImpl>(m_handle);
  return impl->info;
}

} // namespace strobe::gpu
