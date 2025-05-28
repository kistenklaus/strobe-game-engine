#include "./VksContext.hpp"

#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <numeric>
#include <ranges>
#include <vulkan/vulkan_enums.hpp>

#include "renderer/vks/context/VkPhysicalDeviceInfo.hpp"
#include "renderer/vks/context/VksContextExtension.hpp"
#include "spdlog/spdlog.h"

namespace strobe::renderer::vks {

std::expected<VksContext, vk::Result> VksContext::tryCreate(
    strobe::window::Window window, uint32_t apiVersion,
    std::span<VksContextExtension*> extensions,
    std::span<VksContextExtension*> optionalExtensions) {
  auto appInfo = window.getApplicationInfo();
  vk::ApplicationInfo vkAppInfo;
  vkAppInfo.pApplicationName = appInfo->name.data();
  vkAppInfo.applicationVersion = VK_MAKE_VERSION(std::get<0>(appInfo->version),
                                                 std::get<1>(appInfo->version),
                                                 std::get<2>(appInfo->version));
  vkAppInfo.pEngineName = appInfo->engine.name.data();
  vkAppInfo.engineVersion =
      VK_MAKE_VERSION(std::get<0>(appInfo->engine.version),
                      std::get<1>(appInfo->engine.version),
                      std::get<2>(appInfo->engine.version));
  vkAppInfo.apiVersion = apiVersion;

  auto supportedInstanceExtensions = vk::enumerateInstanceExtensionProperties();
  std::vector<const char*> instanceExtensions;
  std::vector<VksContextExtension*> activeOptionalExtensions;
  for (const auto ext : optionalExtensions) {
    activeOptionalExtensions.push_back(ext);
  }

  for (const VksContextExtension* extension : extensions) {
    const auto requiredExtensions = extension->requiredVkInstanceExtensions();
    for (const char* const name : requiredExtensions) {
      bool unsupported = std::ranges::none_of(
          supportedInstanceExtensions,
          [name](const vk::ExtensionProperties& prop) {
            return std::strcmp(name, prop.extensionName) == 0;
          });
      if (unsupported) {
        SPDLOG_ERROR("Extension {} is not supported, but mandatory", name);
        return std::unexpected(vk::Result::eErrorIncompatibleDriver);
      }
      if (std::ranges::none_of(instanceExtensions, [&](const char* x) {
            return std::strcmp(name, x) == 0;
          })) {
        instanceExtensions.push_back(name);
      }
    }
  }
  for (const VksContextExtension* extension : optionalExtensions) {
    const auto requiredExtensions = extension->requiredVkInstanceExtensions();
    for (const char* const name : requiredExtensions) {
      bool unsupported = std::ranges::none_of(
          supportedInstanceExtensions,
          [name](const vk::ExtensionProperties& prop) {
            return std::strcmp(name, prop.extensionName) == 0;
          });
      if (unsupported) {
        std::ranges::remove(activeOptionalExtensions, extension);
        SPDLOG_WARN("Extension {} is not supported", name);
      }
      if (std::ranges::none_of(instanceExtensions, [&](const char* x) {
            return std::strcmp(name, x) == 0;
          })) {
        instanceExtensions.push_back(name);
      }
    }
  }
  auto supportedLayers = vk::enumerateInstanceLayerProperties();
  std::vector<const char*> instanceLayers;
  for (const VksContextExtension* extension : extensions) {
    const auto requiredLayers = extension->requiredVkInstanceLayers();
    for (const char* const name : requiredLayers) {
      bool unsupported = std::ranges::none_of(
          supportedLayers, [name](const vk::LayerProperties& layer) {
            return std::strcmp(name, layer.layerName) == 0;
          });
      if (unsupported) {
        SPDLOG_ERROR("Layer {} is not supported, but mandatory", name);
        return std::unexpected(vk::Result::eErrorIncompatibleDriver);
      }
      if (std::ranges::none_of(instanceLayers, [&](const char* x) {
            return std::strcmp(name, x) == 0;
          })) {
        instanceLayers.push_back(name);
      }
    }
  }
  for (const VksContextExtension* extension : optionalExtensions) {
    const auto requiredLayers = extension->requiredVkInstanceLayers();
    for (const char* const name : requiredLayers) {
      bool unsupported = std::ranges::none_of(
          supportedLayers, [name](const vk::LayerProperties& layer) {
            return std::strcmp(name, layer.layerName) == 0;
          });
      if (unsupported) {
        SPDLOG_WARN("Layer {} is not supported", name);
        std::ranges::remove(activeOptionalExtensions, extension);
        continue;
      }
      if (std::ranges::none_of(instanceLayers, [&](const char* x) {
            return std::strcmp(name, x) == 0;
          })) {
        instanceLayers.push_back(name);
      }
    }
  }

  vk::InstanceCreateInfo instanceCreateInfo;
  instanceCreateInfo.pApplicationInfo = &vkAppInfo;
  instanceCreateInfo.enabledLayerCount = 0;
  instanceCreateInfo.enabledExtensionCount = instanceExtensions.size();
  instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
  instanceCreateInfo.enabledLayerCount = instanceLayers.size();
  instanceCreateInfo.ppEnabledLayerNames = instanceLayers.data();
  void* pNext = nullptr;

  for (VksContextExtension* extension : extensions) {
    pNext = extension->appendVkInstanceCreateInfo(pNext);
  }

  for (VksContextExtension* extension : activeOptionalExtensions) {
    pNext = extension->appendVkInstanceCreateInfo(pNext);
  }

  instanceCreateInfo.pNext = pNext;

  vk::Instance instance;
  {
    vk::Result result =
        vk::createInstance(&instanceCreateInfo, nullptr, &instance);
    if (result != vk::Result::eSuccess) {
      return std::unexpected(result);
    }
  }

  VksContextControlBlock* controlBlock = new VksContextControlBlock;
  controlBlock->dldi =
      vk::detail::DispatchLoaderDynamic{instance, vkGetInstanceProcAddr};

  for (VksContextExtension* extension : extensions) {
    auto lifetime =
        extension->afterInstanceCreation(instance, controlBlock->dldi);
    if (lifetime != nullptr) {
      controlBlock->extensionLifetimes.push_back(std::move(lifetime));
    }
  }
  for (VksContextExtension* extension : activeOptionalExtensions) {
    auto lifetime =
        extension->afterInstanceCreation(instance, controlBlock->dldi);
    if (lifetime != nullptr) {
      controlBlock->extensionLifetimes.push_back(std::move(lifetime));
    }
  }

  vk::SurfaceKHR surface = window.createVkSurface(instance);
  controlBlock->surface = surface;

  // Select physical device!
  auto vkPhysicalDevices = instance.enumeratePhysicalDevices();

  auto physicalDeviceSelectionPipeline =
      vkPhysicalDevices |
      std::views::transform([](const vk::PhysicalDevice& handle) {
        return VksPhysicalDeviceInfo(handle);
      }) |
      std::views::filter([&extensions, surface](
                             const VksPhysicalDeviceInfo& physicalDeviceInfo) {
        return std::ranges::all_of(
            extensions, [&physicalDeviceInfo,
                         surface](const VksContextExtension* extension) {
              return extension->acceptPhysicalDevice(surface,
                                                     physicalDeviceInfo) != 0;
            });
      }) |
      std::views::transform(
          [&extensions, &activeOptionalExtensions,
           surface](const VksPhysicalDeviceInfo& physicalDeviceInfo) {
            auto scores = extensions |
                          std::views::transform(
                              [&physicalDeviceInfo,
                               surface](const VksContextExtension* extension) {
                                return extension->acceptPhysicalDevice(
                                    surface, physicalDeviceInfo);
                              });

            auto scoresOpt =
                activeOptionalExtensions |
                std::views::transform(
                    [&physicalDeviceInfo,
                     surface](const VksContextExtension* optionalExtension) {
                      return optionalExtension->acceptPhysicalDevice(
                          surface, physicalDeviceInfo);
                    });
            auto score = std::accumulate(scores.begin(), scores.end(),
                                         static_cast<uint64_t>(0)) +
                         std::accumulate(scoresOpt.begin(), scoresOpt.end(),
                                         static_cast<uint64_t>(0));

            return std::make_pair(score, physicalDeviceInfo);
          });

  controlBlock->physicalDeviceInfo =
      (*std::ranges::max_element(
           physicalDeviceSelectionPipeline,
           [](const auto& a, const auto& b) { return a.first < b.first; }))
          .second;
  for (std::size_t i = 0; i < activeOptionalExtensions.size(); ++i) {
    if (activeOptionalExtensions[i]->acceptPhysicalDevice(
            surface, controlBlock->physicalDeviceInfo) == 0) {
      activeOptionalExtensions.erase(activeOptionalExtensions.begin() + i);
      i--;
    }
  }

  SPDLOG_INFO("Selected physical device: {}",
              controlBlock->physicalDeviceInfo.deviceName());

  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
  std::vector<float> queuePriorities;
  for (auto& extension : extensions) {
    extension->appendQueueCreateInfo(queueCreateInfos, queuePriorities, surface,
                                     controlBlock->physicalDeviceInfo);
  }

  for (auto& extension : activeOptionalExtensions) {
    extension->appendQueueCreateInfo(queueCreateInfos, queuePriorities, surface,
                                     controlBlock->physicalDeviceInfo);
  }

  vk::DeviceCreateInfo deviceCreateInfo;
  deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
  deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();

  deviceCreateInfo.pNext = nullptr;
  vk::PhysicalDeviceFeatures features;
  for (auto& extension : extensions) {
    extension->enableFeature(controlBlock->physicalDeviceInfo, features);
    deviceCreateInfo.pNext = extension->enableFeature2(
        controlBlock->physicalDeviceInfo, deviceCreateInfo.pNext);
  }
  for (auto& extension : activeOptionalExtensions) {
    extension->enableFeature(controlBlock->physicalDeviceInfo, features);
    deviceCreateInfo.pNext = extension->enableFeature2(
        controlBlock->physicalDeviceInfo, deviceCreateInfo.pNext);
  }
  deviceCreateInfo.pEnabledFeatures = &features;

  std::vector<const char*> deviceExtensionNames;
  for (auto& extension : extensions) {
    for (const char* ext : extension->requiredVkDeviceExtensions()) {
      deviceExtensionNames.push_back(ext);
    }
  }
  for (auto& extension : activeOptionalExtensions) {
    for (const char* ext : extension->requiredVkDeviceExtensions()) {
      deviceExtensionNames.push_back(ext);
    }
  }
  deviceCreateInfo.ppEnabledExtensionNames = deviceExtensionNames.data();
  deviceCreateInfo.enabledExtensionCount = deviceExtensionNames.size();

  vk::Device device;
  {
    vk::Result result = controlBlock->physicalDeviceInfo.handle().createDevice(
        &deviceCreateInfo, nullptr, &device);
    if (result != vk::Result::eSuccess) {
      return std::unexpected(result);
    }
  }

  const auto surfaceSupported =
      controlBlock->physicalDeviceInfo.queueFamilySurfaceSupport(surface);
  const auto queueFamilyProperties =
      controlBlock->physicalDeviceInfo.queueFamilyProperties();
  for (const vk::DeviceQueueCreateInfo& queueCreateInfo : queueCreateInfos) {
    for (std::size_t i = 0; i < queueCreateInfo.queueCount; ++i) {
      vk::Queue queue;
      device.getQueue(queueCreateInfo.queueFamilyIndex, i, &queue);
      const auto props =
          queueFamilyProperties[queueCreateInfo.queueFamilyIndex];
      VksQueueInfo queueInfo(queueCreateInfo.queueFamilyIndex, i, queue);
      if (surfaceSupported[queueCreateInfo.queueFamilyIndex]) {
        controlBlock->presentQueues.push_back(queueInfo);
      }
      if (props.queueFlags & vk::QueueFlagBits::eGraphics) {
        controlBlock->graphicsQueues.push_back(queueInfo);
      }

      if (props.queueFlags & vk::QueueFlagBits::eTransfer) {
        controlBlock->transferQueues.push_back(queueInfo);
      }
      if (props.queueFlags & vk::QueueFlagBits::eCompute) {
        controlBlock->computeQueues.push_back(queueInfo);
      }
    }
  }

  for (auto& extension : extensions) {
    extension->afterDeviceCreation(instance, controlBlock->physicalDeviceInfo,
                                   device);
  }
  for (auto& extension : activeOptionalExtensions) {
    extension->afterDeviceCreation(instance, controlBlock->physicalDeviceInfo,
                                   device);
  }

  return VksContext(instance, device, controlBlock);
}

VksContext::VksContext(vk::Instance instance, vk::Device device,
                       VksContextControlBlock* controlBlock)
    : instance(instance), device(device), m_controlBlock(controlBlock) {}

void VksContext::destroy() {
  for (const auto& extensionLifetime : m_controlBlock->extensionLifetimes) {
    extensionLifetime->destroy(instance, m_controlBlock->dldi);
  }
  m_controlBlock->extensionLifetimes.clear();
  device.destroy();
  instance.destroySurfaceKHR(m_controlBlock->surface);
  instance.destroy();

  delete m_controlBlock;
}

}  // namespace strobe::renderer::vks
