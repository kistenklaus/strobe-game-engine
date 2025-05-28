#pragma once

#include <array>
#include <atomic>
#include <print>
#include <semaphore>

#include "renderer/vks/VksFrame.hpp"

namespace strobe::renderer::vks {

class VksFrameQueue {
 public:
  static constexpr std::size_t QUEUE_SIZE = 2;
  VksFrameQueue()
      : m_emptyFrames(QUEUE_SIZE), m_renderFrames(0), m_stop(false) {}

  // called from beginFrame() frontend.
  VksFrame* acquireEmptyFrame() {
    m_emptyFrames.acquire();
    if (m_stop.load() == true) {
      return nullptr;
    }
    VksFrame* frame = &m_buffer[m_emptyIdx];
    m_emptyIdx = (m_emptyIdx + 1) % QUEUE_SIZE;
    return frame;
  }

  // called from the endFrame() frontend.
  void releaseRenderFrame() { m_renderFrames.release(); }

  // called from the backend to get a new frame
  VksFrame* acquireRenderFrame() {
    m_renderFrames.acquire();
    if (m_stop.load() == true) {
      return nullptr;
    }
    return &m_buffer[m_renderIdx];
  }

  // called from the backend to release the frame.
  void releaseEmptyFrame() {
    m_buffer[m_renderIdx].reset();
    m_renderIdx = (m_renderIdx + 1) % QUEUE_SIZE;
    m_emptyFrames.release();
  }

  void stop() {
    m_stop = true;
    m_emptyFrames.release();
    m_renderFrames.release();
  }

  std::atomic<bool> m_stop;
  std::counting_semaphore<QUEUE_SIZE> m_emptyFrames;
  std::counting_semaphore<QUEUE_SIZE> m_renderFrames;

  std::array<VksFrame, QUEUE_SIZE> m_buffer;

  std::size_t m_emptyIdx = 0;
  std::size_t m_renderIdx = 0;

 private:
};

}  // namespace strobe::renderer::vks
