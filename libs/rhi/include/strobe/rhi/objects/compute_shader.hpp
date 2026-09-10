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
  friend class Object<ComputeShader>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;

public:
  using Object::Object;
  explicit ComputeShader(void *handle) noexcept : Object(handle) {}
  void set_name(const char *name);
};

} // namespace strobe::rhi
