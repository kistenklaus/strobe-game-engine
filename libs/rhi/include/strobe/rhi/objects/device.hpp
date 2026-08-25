#pragma once

#include "strobe/rhi/objects/binary_semaphore.hpp"
#include "strobe/rhi/objects/buffer.hpp"
#include "strobe/rhi/objects/command_pool.hpp"
#include "strobe/rhi/objects/compute_shader.hpp"
#include "strobe/rhi/objects/fence.hpp"
#include "strobe/rhi/objects/fragment_shader.hpp"
#include "strobe/rhi/objects/image.hpp"
#include "strobe/rhi/objects/memory_pool.hpp"
#include "strobe/rhi/objects/queue.hpp"
#include "strobe/rhi/objects/swapchain.hpp"
#include "strobe/rhi/objects/timeline_semaphore.hpp"
#include "strobe/rhi/objects/vertex_shader.hpp"
#include "strobe/rhi/types/device_create_info.hpp"
#include "strobe/rhi/types/queue_flags.hpp"
#include <GLFW/glfw3.h>

namespace strobe::rhi {

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
                             const SwapchainInfo &info = {});
  CommandPool create_cmd_pool(const Queue &queue);
  BinarySemaphore create_binary_semaphore();
  TimelineSemaphore create_timeline_semaphore(uint64_t initalValue = 0);
  Fence create_fence(bool signaled);

  VertexShader create_vertex_shader(const VertexShaderInfo &info);
  FragmentShader create_fragment_shader(const FragmentShaderInfo &info);
  ComputeShader create_compute_shader(const ComputeShaderInfo &info);

  MemoryPool create_memory_pool();

private:
  void *m_handle;
};

} // namespace strobe::rhi
