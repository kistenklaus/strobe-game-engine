#pragma once

#include <cassert>
#include <chrono>
#include "resource/shadersource/ShaderSourceControlBlock.hpp"

namespace strobe::resource {

class ShaderSourceResourceManager;

class ShaderSource {
 public:
   friend class ShaderSourceResourceManager;
  ~ShaderSource() {
    if (m_controlBlock != nullptr) {
      if (m_controlBlock->referenceCounter.dec()) {
        m_controlBlock->lastUsed = std::chrono::high_resolution_clock::now();
      }
    }
  }
  ShaderSource(const ShaderSource& o) noexcept : m_controlBlock(o.m_controlBlock) {
    if (m_controlBlock != nullptr) {
      m_controlBlock->referenceCounter.inc();
    }
  }

  ShaderSource& operator=(const ShaderSource& o) noexcept {
    if (this == &o) {
      return *this;
    }
    if (m_controlBlock != nullptr) {
      m_controlBlock->referenceCounter.dec();
    }
    m_controlBlock = o.m_controlBlock;
    if (m_controlBlock != nullptr) {
      m_controlBlock->referenceCounter.inc();
    }
    return *this;
  }

  ShaderSource(ShaderSource&& o) noexcept : m_controlBlock(o.m_controlBlock) {
    o.m_controlBlock = nullptr;
  }

  ShaderSource& operator=(ShaderSource&& o) noexcept {
    if (this == &o) {
      return *this;
    }
    m_controlBlock = o.m_controlBlock;
    o.m_controlBlock = nullptr;
    return *this;
  }

  std::span<const uint32_t> spirv() const {
    assert(m_controlBlock->size % 4 == 0);
    return std::span<const uint32_t>(m_controlBlock->spirv,
                                      m_controlBlock->size / 4);
  }

 private:
  ShaderSource(detail::ShaderSourceControlBlock* controlBlock) : m_controlBlock(controlBlock) {}
  detail::ShaderSourceControlBlock* m_controlBlock;
};


}
