#include "strobe/rhi/vulkan/device_info/queue_family_properties.hpp"
#include "strobe/core/memory/inplace_monotonic_resource.hpp"

namespace strobe::rhi::vulkan {

Vector<QueueFamilyProperties, strobe::rhi::allocator_ref>
details::query_queue_family_properties(
    VkInstance instance, VkPhysicalDevice physicalDevice,
    const strobe::rhi::allocator_ref &alloc) noexcept {
  using scratch_allocator =
      InplaceMonotonicResource<strobe::rhi::scratch_allocator, 1 << 14>;
  using scratch_allocator_ref = AllocatorReference<scratch_allocator>;

  scratch_allocator scratch{};

  Vector<VkQueueFamilyProperties2, scratch_allocator_ref> native{&scratch};

  {
    ZoneScopedN("vkGetPhysicalDeviceQueueFamilyProperties2");
    uint32_t queueFamilyCount = 0;

    vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &queueFamilyCount,
                                              nullptr);

    native.resize(queueFamilyCount, {});

    for (auto &props : native) {
      props.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
      props.pNext = nullptr;

      // Later extend here.
      // Any extension structs can be allocated from scratch.
    }

    vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &queueFamilyCount,
                                              native.data());

    native.resize(queueFamilyCount);
  }

  Vector<QueueFamilyProperties, strobe::rhi::allocator_ref>
      queueFamilyProperties{alloc};
  queueFamilyProperties.reserve(native.size());

  for (uint32_t qfi = 0; qfi < native.size(); ++qfi) {
    const auto &props = native[qfi];

    bool presentationSupport;
    {
      ZoneScopedN("glfwGetPhysicalDevicePresentationSupport");
      presentationSupport = glfwGetPhysicalDevicePresentationSupport(
                                instance, physicalDevice, qfi) == GLFW_TRUE;
    }

    queueFamilyProperties.push_back(QueueFamilyProperties{
        .queueFlags = props.queueFamilyProperties.queueFlags,

        .queueCount = props.queueFamilyProperties.queueCount,

        .timestampValidBits = props.queueFamilyProperties.timestampValidBits,

        .minImageTransferGranularity =
            props.queueFamilyProperties.minImageTransferGranularity,

        .presentationSupport = presentationSupport,
    });
  }

  return queueFamilyProperties;
}
} // namespace strobe::rhi::vulkan
