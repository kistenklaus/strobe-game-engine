#pragma once

#include "strobe/gpu/device/binary_semaphore.hpp"
#include "strobe/gpu/device/buffer.hpp"
#include "strobe/gpu/device/command_pool.hpp"
#include "strobe/gpu/device/device_create_info.hpp"
#include "strobe/gpu/device/device_info.hpp"
#include "strobe/gpu/device/fence.hpp"
#include "strobe/gpu/device/fragment_shader.hpp"
#include "strobe/gpu/device/image.hpp"
#include "strobe/gpu/device/memory_pool.hpp"
#include "strobe/gpu/device/queue.hpp"
#include "strobe/gpu/device/queue_flags.hpp"
#include "strobe/gpu/device/swapchain.hpp"
#include "strobe/gpu/device/timeline_semaphore.hpp"
#include "strobe/gpu/device/vertex_shader.hpp"
#include <GLFW/glfw3.h>

namespace strobe::gpu {

class Device {
  friend struct CommandPoolImpl;
  friend class CommandPool;
  friend struct ImageImpl;
  friend struct ImageViewImpl;
  friend class Image;
  friend struct SwapchainGenerationImpl;
  friend struct SwapchainImpl;
  friend struct SurfaceImpl;
  friend class Swapchain;
  friend struct BinarySemaphoreImpl;
  friend struct FenceImpl;
  friend class Fence;
  friend struct TimelineSemaphoreImpl;
  friend class TimelineSemaphore;

public:
  Device(const DeviceCreateInfo &createInfo);
  Device(const Device &) noexcept;
  Device(Device &&) noexcept;
  Device &operator=(const Device &) noexcept;
  Device &operator=(Device &&) noexcept;
  ~Device() noexcept;

  Queue get_queue(QueueFlags flags);
  Swapchain create_swapchain(GLFWwindow *window,
                             const SwapchainCreateInfo &createInfo = {});
  CommandPool create_cmd_pool(const Queue &queue);
  Image create_image(const ImageCreateInfo &);
  BinarySemaphore create_binary_semaphore();
  TimelineSemaphore create_timeline_semaphore(uint64_t initalValue = 0);
  Fence create_fence(bool signaled);
  VertexShader create_vertex_shader(const VertexShaderCreateInfo &info);
  FragmentShader create_fragment_shader(const FragmentShaderCreateInfo &info);

  MemoryPool create_memory_pool();
  // NOTE: we may later add this back in (backed by a internal memory pool or something)
  // Buffer create_buffer(const BufferCreateInfo &info);

  const DeviceInfo &info() const noexcept;

private:
  void *m_handle;
};

} // namespace strobe::gpu
