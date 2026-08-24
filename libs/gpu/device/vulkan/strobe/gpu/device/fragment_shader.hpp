#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/gpu/device/push_constant_range.hpp"
#include <cstdint>

namespace strobe::gpu {

struct FragmentShaderInfo {
  span<const uint32_t> spirv = {};
  span<const PushConstantRange> pushConstantRange = {};
};

struct FragmentShader {
  friend class Device;
  friend class CommandBuffer;
  friend struct CommandBufferImpl;

public:
  FragmentShader() noexcept : m_handle(nullptr) {}
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

private:
  FragmentShader(void *handle) noexcept : m_handle(handle) {}
  void *m_handle;
};

} // namespace strobe::gpu
