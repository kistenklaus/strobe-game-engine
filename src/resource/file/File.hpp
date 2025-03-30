#pragma once

#include <chrono>
#include <cstdint>
#include <ostream>
#include <print>
#include <span>
#include <string_view>

#include "resource/file/FileControlBlock.hpp"


namespace strobe::resource {

class FileResourceManager;

class File {
 public:
   friend class FileResourceManager;
  ~File() {
    if (m_controlBlock != nullptr) {
      if (m_controlBlock->referenceCounter.dec()) {
        m_controlBlock->lastUsed = std::chrono::high_resolution_clock::now();
      }
    }
  }
  File(const File& o) noexcept : m_controlBlock(o.m_controlBlock) {
    if (m_controlBlock != nullptr) {
      m_controlBlock->referenceCounter.inc();
    }
  }

  File& operator=(const File& o) noexcept {
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

  File(File&& o) noexcept : m_controlBlock(o.m_controlBlock) {
    o.m_controlBlock = nullptr;
  }

  File& operator=(File&& o) noexcept {
    if (this == &o) {
      return *this;
    }
    m_controlBlock = o.m_controlBlock;
    o.m_controlBlock = nullptr;
    return *this;
  }

  std::span<const std::byte> bytes() const {
    return std::span<const std::byte>(m_controlBlock->data,
                                      m_controlBlock->size);
  }

  std::string_view text() const {
    return std::string_view(reinterpret_cast<const char*>(m_controlBlock->data),
                            m_controlBlock->size);
  }

 private:
  File(detail::FileControlBlock* controlBlock) : m_controlBlock(controlBlock) {}
  detail::FileControlBlock* m_controlBlock;
};

}  // namespace strobe::resource
