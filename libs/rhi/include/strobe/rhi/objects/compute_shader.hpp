#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/rhi/objects/object.hpp"
#include <cstdint>

namespace strobe::rhi {

struct ComputeShaderInfo {
  span<const uint32_t> spirv = {};
};


/**
 * \ingroup rhi
 * \brief todo
 */
class ComputeShader : public Object<ComputeShader> {
  friend class Device;
  friend class CommandBuffer;
  friend struct CommandBufferImpl;

public:
  explicit ComputeShader(void *handle) noexcept : Object(handle) {}
  ComputeShader() noexcept : Object(nullptr) {}
  ComputeShader(const ComputeShader &) noexcept;
  ComputeShader(ComputeShader &&) noexcept;
  ComputeShader &operator=(const ComputeShader &) noexcept;
  ComputeShader &operator=(ComputeShader &&) noexcept;
  ~ComputeShader() noexcept;
  void set_name(const char *name);
};

} // namespace strobe::rhi
