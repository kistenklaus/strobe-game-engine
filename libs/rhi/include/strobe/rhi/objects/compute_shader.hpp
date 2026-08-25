#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/rhi/types/push_constant_range.hpp"

namespace strobe::rhi {

struct ComputeShaderInfo {
  span<const uint32_t> spirv = {};
  span<const PushConstantRange> pushConstantRange = {};
};

class ComputeShader {
  friend class Device;
  friend class CommandBuffer;
  friend struct CommandBufferImpl;

public:
  ComputeShader() noexcept : m_handle(nullptr) {}
  ComputeShader(const ComputeShader &) noexcept;
  ComputeShader(ComputeShader &&) noexcept;
  ComputeShader &operator=(const ComputeShader &) noexcept;
  ComputeShader &operator=(ComputeShader &&) noexcept;
  ~ComputeShader() noexcept;
  explicit operator bool() const noexcept { return m_handle != nullptr; }
  friend bool operator==(const ComputeShader &lhs,
                         const ComputeShader &rhs) noexcept {
    return lhs.m_handle == rhs.m_handle;
  }
  friend bool operator!=(const ComputeShader &lhs,
                         const ComputeShader &rhs) noexcept {
    return lhs.m_handle == rhs.m_handle;
  }
  void set_name(const char *name);

private:
  ComputeShader(void *handle) noexcept : m_handle(handle) {}
  void *m_handle;
};

} // namespace strobe::rhi
