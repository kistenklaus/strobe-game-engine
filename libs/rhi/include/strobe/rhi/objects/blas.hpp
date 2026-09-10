#pragma once

#include "strobe/rhi/objects/object.hpp"
namespace strobe::rhi {
/**
 * \ingroup rhi
 * \brief Bottom Level Acceleration Structure
 *
 * Blas is a BVH acceleration structure, containing triangles or AABBs.
 */
struct Blas : public Object<Blas> {
  friend class Object<Blas>;
  static void pin(void *handle) noexcept;
  static void unpin(void *handle) noexcept;
  using Object::Object;
};

} // namespace strobe::rhi
