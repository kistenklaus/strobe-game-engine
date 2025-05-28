#pragma once

#include <cstdint>
#include <memory>
#include <vulkan/vulkan.hpp>

namespace strobe::renderer {

namespace detail {

struct VksQueueInfoStorage {
  std::uint32_t queueFamilyIndex;
  std::uint32_t queueIndex;
  vk::Queue queue;
};

};  // namespace detail

class VksQueueInfo {
 public:
  VksQueueInfo() : m_controlBlock(nullptr) {}
  VksQueueInfo(std::uint32_t queueFamilyIndex, std::uint32_t queueIndex,
               vk::Queue handle)
      : m_controlBlock(std::make_shared<detail::VksQueueInfoStorage>()) {
    m_controlBlock->queueFamilyIndex = queueFamilyIndex;
    m_controlBlock->queueIndex = queueIndex;
    m_controlBlock->queue = handle;
  }

  uint32_t queueFamilyIndex() const { return m_controlBlock->queueFamilyIndex; }

  uint32_t queueIndex() const { return m_controlBlock->queueIndex; }

  vk::Queue handle() const { return m_controlBlock->queue; }

 private:
  std::shared_ptr<detail::VksQueueInfoStorage> m_controlBlock;
};

}  // namespace strobe::renderer
