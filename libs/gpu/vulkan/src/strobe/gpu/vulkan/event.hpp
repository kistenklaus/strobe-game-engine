#pragma once

#include "strobe/gpu/vulkan/context/context.hpp"

#include <vulkan/vulkan_core.h>

namespace strobe::gpu::vulkan {

struct Event {
  VkEvent handle = VK_NULL_HANDLE;

  explicit operator bool() const noexcept { return handle != VK_NULL_HANDLE; }
};

struct EventInfo {
  VkEventCreateFlags flags = 0;
};

Event create_event(Context *context, const EventInfo &info = {});
void destroy_event(Context *context, Event event) noexcept;
bool is_event_signaled(Context *context, Event event) noexcept;
void set_event(Context *context, Event event);
void reset_event(Context *context, Event event);

} // namespace strobe::gpu::vulkan
