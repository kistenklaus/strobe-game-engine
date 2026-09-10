#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/rhi/objects/object.hpp"
#include <cstdint>

namespace strobe::rhi {

struct FragmentShaderInfo {
  span<const uint32_t> spirv = {};
};

/**
 * \ingroup rhi
 * \brief todo
 */
class FragmentShader : public Object<FragmentShader> {
  friend class Object<FragmentShader>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;

public:
  using Object::Object;
};

} // namespace strobe::rhi
