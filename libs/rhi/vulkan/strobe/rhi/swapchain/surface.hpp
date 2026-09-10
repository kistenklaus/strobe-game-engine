#pragma once

#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/vulkan/surface.hpp"
namespace strobe::rhi {

class Surface : public Object<Surface> {
  friend class Object<Surface>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;

public:
  using Object::Object;

  vulkan::Surface get() const noexcept;
  vulkan::Context *ctx() const noexcept;
  const Context &context() const noexcept;
};

} // namespace strobe::rhi
