#include "strobe/gpu/vulkan/event.hpp"

#include <cassert>
#include <stdexcept>

namespace strobe::gpu::vulkan {

Event create_event(Context *context, const EventInfo &info) {
  assert(context != nullptr);
  const VkEventCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO,
      .pNext = nullptr,
      .flags = info.flags,
  };
  Event event{};
  const VkResult result = vkCreateEvent(context->device(), &createInfo,
                                        context->driver_alloc(), &event.handle);
  if (result != VK_SUCCESS) {
    throw std::runtime_error{"Failed to create Vulkan event"};
  }
  return event;
}

void destroy_event(Context *context, Event event) noexcept {
  assert(context != nullptr);
  assert(event);
  vkDestroyEvent(context->device(), event.handle, context->driver_alloc());
}

bool is_event_signaled(Context *context, Event event) noexcept {
  assert(context != nullptr);
  assert(event);
  const VkResult result = vkGetEventStatus(context->device(), event.handle);
  assert(result == VK_EVENT_SET || result == VK_EVENT_RESET);
  return result == VK_EVENT_SET;
}

void set_event(Context *context, Event event) {
  assert(context != nullptr);
  assert(event);
  const VkResult result = vkSetEvent(context->device(), event.handle);
  if (result != VK_SUCCESS) {
    throw std::runtime_error{"Failed to set Vulkan event"};
  }
}

void reset_event(Context *context, Event event) {
  assert(context != nullptr);
  assert(event);
  const VkResult result = vkResetEvent(context->device(), event.handle);
  if (result != VK_SUCCESS) {
    throw std::runtime_error{"Failed to reset Vulkan event"};
  }
}

} // namespace strobe::gpu::vulkan
