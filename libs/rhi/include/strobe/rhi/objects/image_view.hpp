#pragma once

#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/types/format.hpp"

namespace strobe::rhi {

// fwd decl.
class Image;

/**
 * \ingroup rhi
 * \brief todo
 */
class ImageView : public Object<ImageView> {
  friend class Object<ImageView>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;

public:
  using Object::Object;
  Format format() const noexcept;
  const Image &image() const noexcept;
  explicit ImageView(void *handle) noexcept : Object(handle) {}
};

} // namespace strobe::rhi

// complete fwd definition.
#include "strobe/rhi/objects/image.hpp"
