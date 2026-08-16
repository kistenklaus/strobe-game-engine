#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/gpu/device/push_constant_range.hpp"
#include <cstdint>

namespace strobe::gpu {

struct FragmentShaderCreateInfo {
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

private:
  FragmentShader(void *handle) noexcept : m_handle(handle) {}
  void *m_handle;
};

} // namespace strobe::gpu
