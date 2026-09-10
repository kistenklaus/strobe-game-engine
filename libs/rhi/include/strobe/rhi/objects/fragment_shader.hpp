#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/rhi/objects/object.hpp"
#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief todo
 */
struct FragmentShaderInfo {
  span<const uint32_t> spirv = {};
};

class FragmentShader : public Object<FragmentShader> {
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
};

} // namespace strobe::rhi
