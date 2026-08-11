#include "strobe/gpu/vulkan/context/instance.hpp"

namespace strobe::gpu::vulkan {

namespace details {

static constexpr uint32_t MAX_API_VERSION = VK_API_VERSION_1_4;

static uint32_t query_api_version() noexcept {
  uint32_t version;
  const VkResult result = vkEnumerateInstanceVersion(&version);
  if (result != VK_SUCCESS) {
    return VK_API_VERSION_1_0;
  }
  return std::min(version, MAX_API_VERSION);
}

static bool
check_instance_extension_support(span<const VkExtensionProperties> supported,
                                 const char *extension_name, feature request) {

  if (request == disable) {
    return false;
  }
  auto it = std::ranges::find_if(
      supported,
      [name = extension_name](const VkExtensionProperties &props) -> bool {
        return std::strcmp(props.extensionName, name) == 0;
      });
  if (it == supported.end()) {
    if (request == required) {
      throw std::runtime_error{fmt::format("Required Vulkan instance extension "
                                           "'{}' is not supported",
                                           extension_name)};
    }
    return false;
  }
  return true;
}

static bool
check_instance_layer_support(span<const VkLayerProperties> supported,
                             const char *layer_name, feature request) {
  if (request == disable) {
    return false;
  }
  const auto it = std::ranges::find_if(
      supported, [layer_name](const VkLayerProperties &properties) {
        return std::strcmp(properties.layerName, layer_name) == 0;
      });
  if (it == supported.end()) {
    if (request == required) {
      throw std::runtime_error{fmt::format("Required Vulkan instance layer "
                                           "'{}' is not supported",
                                           layer_name)};
    }
    return false;
  }
  return true;
}

} // namespace details

VkInstance create_instance(const ContextCreateInfo *info,
                           ContextProperties *props, DriverAlloc *alloc) {
  using scatch_allocator =
      InplaceMonotonicResource<strobe::Mallocator, 1 << 14>;
  using scatch_traits = AllocatorTraits<scatch_allocator>;
  using scatch_allocator_ref = AllocatorReference<scatch_allocator>;
  scatch_allocator scatch{};

  const uint32_t api_version = details::query_api_version();
  props->api_version = api_version;

  // query supported instance extensions.
  Vector<VkExtensionProperties, scatch_allocator_ref> supported_extensions{
      &scatch};
  {
    uint32_t extension_count = 0;
    VkResult result = vkEnumerateInstanceExtensionProperties(
        nullptr, &extension_count, nullptr);
    if (result != VK_SUCCESS) {
      throw std::runtime_error{
          "Failed to query Vulkan instance extension count"};
    }
    do {
      supported_extensions.resize(extension_count);
      result = vkEnumerateInstanceExtensionProperties(
          nullptr, &extension_count, supported_extensions.data());
    } while (result == VK_INCOMPLETE);
    if (result != VK_SUCCESS) {
      throw std::runtime_error{"Failed to query Vulkan instance extensions"};
    }
    supported_extensions.resize(extension_count);
  }
  // query supported layers
  Vector<VkLayerProperties, scatch_allocator_ref> supported_layers{&scatch};
  {
    std::uint32_t layer_count = 0;
    VkResult result = vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
    if (result != VK_SUCCESS) {
      throw std::runtime_error{"Failed to query Vulkan instance layer count."};
    }
    do {
      supported_layers.resize(layer_count);
      result = vkEnumerateInstanceLayerProperties(&layer_count,
                                                  supported_layers.data());
    } while (result == VK_INCOMPLETE);
    if (result != VK_SUCCESS) {
      throw std::runtime_error{"Failed to query Vulkan instance layers."};
    }
    supported_layers.resize(layer_count);
  }

  Vector<const char *, scatch_allocator_ref> extensions{&scatch};
  extensions.reserve(1 << 5);

  Vector<const char *, scatch_allocator_ref> layers{&scatch};
  layers.reserve(1 << 4);

  void *pNext = nullptr;

  if (details::check_instance_extension_support(
          supported_extensions, "VK_EXT_debug_utils", info->debug_utils) &&
      details::check_instance_layer_support(
          supported_layers, "VK_LAYER_KHRONOS_validation", info->debug_utils)) {
    extensions.emplace_back("VK_EXT_debug_utils");
    layers.emplace_back("VK_LAYER_KHRONOS_validation");
    props->debug_utils = true;
    auto *messenger =
        scatch_traits::template allocate<VkDebugUtilsMessengerCreateInfoEXT>(
            scatch);
    *messenger = debug_messenger_create_info();
    messenger->pNext = pNext;
    pNext = messenger;
  }
  if (info->surface != disable) {
    std::uint32_t count = 0;
    const char **required_extensions =
        glfwGetRequiredInstanceExtensions(&count);
    if (required_extensions == nullptr) {
      if (info->surface == required) {
        throw std::runtime_error{
            "GLFW could not provide Vulkan surface extensions"};
      }
    } else {
      bool supported = true;
      for (std::uint32_t i = 0; i < count; ++i) {
        if (!details::check_instance_extension_support(
                supported_extensions, required_extensions[i], info->surface)) {
          supported = false;
          break;
        }
      }
      if (supported) {
        props->surface = true;
        for (std::uint32_t i = 0; i < count; ++i) {
          extensions.emplace_back(required_extensions[i]);
        }
      }
    }
  }

  VkApplicationInfo appInfo{
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pNext = nullptr,
      .pApplicationName = nullptr,
      .applicationVersion = VK_MAKE_VERSION(0, 0, 0),
      .pEngineName = "strobe",
      .engineVersion = VK_MAKE_VERSION(0, 0, 0),
      .apiVersion = api_version,
  };

  VkInstanceCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext = pNext,
      .flags = 0,
      .pApplicationInfo = &appInfo,
      .enabledLayerCount = static_cast<uint32_t>(layers.size()),
      .ppEnabledLayerNames = layers.data(),
      .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
      .ppEnabledExtensionNames = extensions.data(),
  };

  VkInstance instance = VK_NULL_HANDLE;
  VkResult result =
      vkCreateInstance(&createInfo, alloc->callbacks(), &instance);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to create vulkan instance");
  }
  return instance;
}
} // namespace strobe::gpu::vulkan
