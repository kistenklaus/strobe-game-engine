#pragma once

#include <cassert>
#include <ostream>
#include <print>

#include "renderer/resources.hpp"
#include "renderer/vks/VksFrameQueue.hpp"
#include "renderer/vks/resources/ResourcePools.hpp"
namespace strobe::renderer::vks {

class VksFrontend {
 public:
  explicit VksFrontend(VksFrameQueue* frameQueue,
                       VksResourcePools* resourcePools)
      : m_frameQueue(frameQueue), m_resourcePools(resourcePools) {}

  // can block if we cannot acquire a new frame.
  void beginFrame() { m_frame = m_frameQueue->acquireEmptyFrame(); }

  ResourceHandle createText(std::string_view text) {
    return m_resourcePools->text.create(text);
  }
  void drawText(ResourceHandle text) {
    m_resourcePools->text.pin(text);
    assert(m_frame != nullptr);
    m_frame->drawCommands.push_back(reinterpret_cast<VksTextResource*>(text));
  }

  void destroyText(ResourceHandle text) {
    m_resourcePools->text.release(text);
  }

  // can never block. Just marks the frame as done!
  void endFrame() {
    m_frameQueue->releaseRenderFrame();
    m_frame = nullptr;
  }

 private:
  VksFrameQueue* m_frameQueue;
  VksResourcePools* m_resourcePools;

  VksFrame* m_frame = nullptr;
};

}  // namespace strobe::renderer::vks
