#include "strobe/rhi/vulkan/device_info/device_extensions.hpp"

#include "strobe/core/memory/AllocatorReference.hpp"
#include "strobe/core/memory/inplace_monotonic_resource.hpp"

namespace strobe::rhi::vulkan {

Vector<DeviceExtension, strobe::rhi::allocator_ref>
details::query_device_extensions(VkPhysicalDevice physicalDevice,
                                 const strobe::rhi::allocator_ref &alloc) {
  using scratch_allocator =
      InplaceMonotonicResource<strobe::rhi::scratch_allocator, 1 << 10>;
  using scratch_allocator_ref = AllocatorReference<scratch_allocator>;
  scratch_allocator scratch{};

  Vector<VkExtensionProperties, scratch_allocator_ref> native{&scratch};
  while (true) {
    std::uint32_t count = 0;
    VkResult result = vkEnumerateDeviceExtensionProperties(
        physicalDevice, nullptr, &count, nullptr);
    if (result != VK_SUCCESS) {
      throw std::runtime_error{"Failed to enumerate device extensions"};
    }
    native.resize(count);
    result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr,
                                                  &count, native.data());
    native.resize(count);
    if (result == VK_SUCCESS) {
      break;
    }
    if (result != VK_INCOMPLETE) {
      throw std::runtime_error{"Failed to enumerate device extensions"};
    }
  }
  Vector<DeviceExtension, strobe::rhi::allocator_ref> extensions{alloc};
  extensions.reserve(native.size());
  for (const auto &extension : native) {
    extensions.push_back(DeviceExtension{
        .name = {extension.extensionName, alloc},
        .specVersion = extension.specVersion,
    });
  }
  return extensions;
}
bool details::supports_extension(span<const DeviceExtension> extensions,
                                 const char *required) {
  return std::ranges::any_of(
      extensions, [required](const DeviceExtension &extension) noexcept {
        return std::string_view{extension.name.c_str()} ==
               std::string_view{required};
      });
}

} // namespace strobe::rhi::vulkan
