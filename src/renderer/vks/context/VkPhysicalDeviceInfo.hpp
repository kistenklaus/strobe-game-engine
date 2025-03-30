#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <functional>
#include <memory>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace strobe::renderer::vks {

namespace detail {

struct VksPhysicalDeviceInfoStorage {
  vk::PhysicalDevice physicalDevice = nullptr;
  std::unique_ptr<vk::PhysicalDeviceProperties> properties = nullptr;
  std::vector<std::unique_ptr<VkBaseOutStructure>> properties2;

  std::unique_ptr<vk::PhysicalDeviceFeatures> features = nullptr;
  std::vector<std::unique_ptr<VkBaseOutStructure>> features2;

  std::vector<vk::QueueFamilyProperties> queueFamilyProperties;

  std::vector<uint8_t> queueFamilySurfaceSupport;
  std::unique_ptr<vk::SurfaceCapabilitiesKHR> surfaceCapabilities;
  std::vector<vk::SurfaceFormatKHR> surfaceFormats;
  std::vector<vk::PresentModeKHR> surfacePresentModes;
};

};  // namespace detail

class VksPhysicalDeviceInfo {
 public:
  VksPhysicalDeviceInfo() : m_controlBlock(nullptr) {}
  VksPhysicalDeviceInfo(vk::PhysicalDevice handle)
      : m_controlBlock(
            std::make_shared<detail::VksPhysicalDeviceInfoStorage>()) {
    m_controlBlock->physicalDevice = handle;
  }

  const vk::PhysicalDeviceProperties& properties() const {
    if (m_controlBlock->properties == nullptr) {
      m_controlBlock->properties =
          std::make_unique<vk::PhysicalDeviceProperties>(
              m_controlBlock->physicalDevice.getProperties());
    }
    return *m_controlBlock->properties;
  }

  template <typename Properties>
  const Properties& properties() const {
    constexpr vk::StructureType sType = Properties::structureType;
    auto it = std::ranges::find_if(
        m_controlBlock->properties2,
        [sType](const auto& prop2) { return prop2.sType == sType; });
    if (it == m_controlBlock->properties2.end()) {
      auto prop = std::make_unique<Properties>();
      prop->sType = sType;
      VkPhysicalDeviceProperties2 prop2;
      prop2.pNext = prop.get();
      prop2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
      vkGetPhysicalDeviceProperties2(m_controlBlock->physicalDevice, &prop2);
      if (m_controlBlock->properties == nullptr) {
        m_controlBlock->properties =
            std::make_unique<vk::PhysicalDeviceProperties>(prop2.properties);
      }
      m_controlBlock->properties2.push_back(std::move(prop));
      return *reinterpret_cast<const Properties*>(
          m_controlBlock->properties2.back().get());
    } else {
      return *(*it);
    }
  }

  std::string_view deviceName() const {
    auto props = properties();
    return std::string_view(props.deviceName);
  }

  const vk::PhysicalDeviceFeatures& features() const {
    if (m_controlBlock->features == nullptr) {
      m_controlBlock->features = std::make_unique<vk::PhysicalDeviceFeatures>(
          m_controlBlock->physicalDevice.getFeatures());
    }
    return *m_controlBlock->features;
  }

  template <typename Features>
  const Features& features() const {
    constexpr vk::StructureType sType = Features::structureType;
    auto it = std::ranges::find_if(
        m_controlBlock->features2,
        [sType](const auto& feature2) { return feature2.sType == sType; });
    if (it == m_controlBlock->features2.end()) {
      auto feature = std::make_unique<Features>();
      feature->sType = sType;
      VkPhysicalDeviceFeatures2 features2;
      features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
      features2.pNext = feature.get();
      vkGetPhysicalDeviceFeatures2(m_controlBlock->physicalDevice, &features2);
      if (m_controlBlock->features == nullptr) {
        m_controlBlock->features =
            std::make_unique<vk::PhysicalDeviceFeatures>(features2.features);
      }
      m_controlBlock->features2.push_back(std::move(feature));
      return *m_controlBlock->features2.back();
    } else {
      return *(*it);
    }
  }

  std::span<const vk::QueueFamilyProperties> queueFamilyProperties() const {
    if (m_controlBlock->queueFamilyProperties.empty()) {
      m_controlBlock->queueFamilyProperties =
          m_controlBlock->physicalDevice.getQueueFamilyProperties();
    }
    return m_controlBlock->queueFamilyProperties;
  }

  std::span<const bool> queueFamilySurfaceSupport(
      vk::SurfaceKHR surface) const {
    if (m_controlBlock->queueFamilySurfaceSupport.empty()) {
      auto queueFamilies = queueFamilyProperties();

      for (std::size_t familyIndex = 0; familyIndex < queueFamilies.size();
           ++familyIndex) {
        bool sup = m_controlBlock->physicalDevice.getSurfaceSupportKHR(
            static_cast<std::uint32_t>(familyIndex), surface);
        m_controlBlock->queueFamilySurfaceSupport.push_back(sup);
      }
    }
    return std::span<const bool>(
        reinterpret_cast<const bool*>(
            m_controlBlock->queueFamilySurfaceSupport.data()),
        m_controlBlock->queueFamilySurfaceSupport.size());
  }

  const vk::SurfaceCapabilitiesKHR& surfaceCapabilities(
      vk::SurfaceKHR surface) const {
    if (m_controlBlock->surfaceCapabilities == nullptr) {
      m_controlBlock->surfaceCapabilities =
          std::make_unique<vk::SurfaceCapabilitiesKHR>(
              m_controlBlock->physicalDevice.getSurfaceCapabilitiesKHR(
                  surface));
    }
    return *m_controlBlock->surfaceCapabilities;
  }

  std::span<const vk::SurfaceFormatKHR> surfaceFormats(vk::SurfaceKHR surface) const {
    if (m_controlBlock->surfaceFormats.empty()) {
      m_controlBlock->surfaceFormats = m_controlBlock->physicalDevice.getSurfaceFormatsKHR(surface);
    }
    return m_controlBlock->surfaceFormats;
  }

  std::span<const vk::PresentModeKHR> surfacePresentModes(vk::SurfaceKHR surface) const {
    if (m_controlBlock->surfacePresentModes.empty()) {
      m_controlBlock->surfacePresentModes = m_controlBlock->physicalDevice.getSurfacePresentModesKHR(surface);
    }
    return m_controlBlock->surfacePresentModes;
  }


  void releaseCached() const {
    m_controlBlock->properties.reset();
    m_controlBlock->properties2.clear();
    m_controlBlock->features.reset();
    m_controlBlock->features2.clear();
    m_controlBlock->queueFamilyProperties.clear();
    m_controlBlock->queueFamilySurfaceSupport.clear();
    m_controlBlock->surfaceCapabilities.reset();
    m_controlBlock->surfaceFormats.clear();
  }

  vk::PhysicalDevice handle() const { return m_controlBlock->physicalDevice; }

 private:
  std::shared_ptr<detail::VksPhysicalDeviceInfoStorage> m_controlBlock;
};

}  // namespace strobe::renderer::vks
