#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/gpu/device/push_constant_range.hpp"
#include "strobe/gpu/device/shader_stage.hpp"
namespace strobe::gpu {

struct VertexShaderInfo {
  span<const uint32_t> spirv = {};
  ShaderStage nextStage = ShaderStage::fragment;
  span<const PushConstantRange> pushConstantRange = {};
};

struct VertexShader {
  friend class Device;
  friend class CommandBuffer;
  friend struct CommandBufferImpl;

public:
  VertexShader() noexcept : m_handle(nullptr) {}
  VertexShader(const VertexShader &) noexcept;
  VertexShader(VertexShader &&) noexcept;
  VertexShader &operator=(const VertexShader &) noexcept;
  VertexShader &operator=(VertexShader &&) noexcept;
  ~VertexShader() noexcept;
  explicit operator bool() const noexcept { return m_handle != nullptr; }
  friend bool operator==(const VertexShader &lhs, const VertexShader &rhs) {
    return lhs.m_handle == rhs.m_handle;
  }
  friend bool operator!=(const VertexShader &lhs, const VertexShader &rhs) {
    return lhs.m_handle != rhs.m_handle;
  }

private:
  VertexShader(void *handle) noexcept : m_handle(handle) {}
  void *m_handle;
};

} // namespace strobe::gpu
