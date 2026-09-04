#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/types/shader_stage.hpp"

namespace strobe::rhi {

struct VertexShaderInfo {
  span<const uint32_t> spirv = {};
  ShaderStage nextStage = ShaderStage::fragment;
};

struct VertexShader : Object<VertexShader> {
  friend class Device;
  friend class CommandBuffer;
  friend struct CommandBufferImpl;

public:
  explicit VertexShader(void *handle) noexcept : Object(handle) {}
  VertexShader() noexcept : Object(nullptr) {}
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
};

} // namespace strobe::rhi
