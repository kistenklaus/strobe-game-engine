#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/rhi/objects/object.hpp"
#include <cstdint>

namespace strobe::rhi {

struct FragmentShaderInfo {
  span<const uint32_t> spirv = {};
};

class FragmentShader : Object<FragmentShader> {
  friend class Device;
  friend class CommandBuffer;
  friend struct CommandBufferImpl;

public:
  explicit FragmentShader(void *handle) noexcept : Object(handle) {}
  FragmentShader() noexcept : Object(nullptr) {}
  FragmentShader(const FragmentShader &) noexcept;
  FragmentShader(FragmentShader &&) noexcept;
  FragmentShader &operator=(const FragmentShader &) noexcept;
  FragmentShader &operator=(FragmentShader &&) noexcept;
  ~FragmentShader() noexcept;
  explicit operator bool() const noexcept { return m_handle; }
  friend bool operator==(const FragmentShader &lhs,
                         const FragmentShader &rhs) noexcept {
    return lhs.m_handle == rhs.m_handle;
  }
  friend bool operator!=(const FragmentShader &lhs,
                         const FragmentShader &rhs) noexcept {
    return lhs.m_handle != rhs.m_handle;
  }
};

} // namespace strobe::rhi
