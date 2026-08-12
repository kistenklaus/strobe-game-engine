#pragma once

#include "strobe/gpu/device/command_pool.hpp"
#include "strobe/gpu/device/device_create_info.hpp"
#include "strobe/gpu/device/device_info.hpp"
#include "strobe/gpu/device/image.hpp"
#include "strobe/gpu/device/queue.hpp"
#include "strobe/gpu/device/queue_flags.hpp"
#include "strobe/gpu/device/swapchain.hpp"
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


public:
  Device(const DeviceCreateInfo &createInfo);
  Device(const Device &) noexcept;
  Device(Device &&) noexcept;
  Device &operator=(const Device &) noexcept;
  Device &operator=(Device &&) noexcept;
  ~Device() noexcept;

  Queue create_queue(QueueFlags flags);
  Swapchain create_swapchain(GLFWwindow *window,
                             const SwapchainCreateInfo &createInfo = {});

  CommandPool create_cmd_pool(const Queue &queue);
  Image create_image(const ImageCreateInfo &);

  // ImageView create_image_view(Image image, const ImageViewCreateInfo &createInfo = {});

  const DeviceInfo &info() const noexcept;

private:
  void *m_handle;
};

} // namespace strobe::gpu
