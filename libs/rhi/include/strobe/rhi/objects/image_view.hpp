#pragma once

#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/types/format.hpp"

namespace strobe::rhi {

// fwd decl.
class Image;

/**
 * \ingroup rhi
 * \brief Image view object.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \code{.cpp}
 * class ImageView : public Object<ImageView>;
 * \endcode
 *
 * Represents a typed view into an Image.
 */
class ImageView : public Object<ImageView> {
  friend class Object<ImageView>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;

public:
  using Object::Object;

  /**
   * \brief Gets view format.
   *
   * Returns the format used by this image view.
   *
   * \return Image view format.
   */
  Format format() const noexcept;

  /**
   * \brief Gets source image.
   *
   * Returns the image referenced by this view.
   *
   * \return Referenced image.
   */
  const Image &image() const noexcept;

  /**
   * \brief Wraps image view.
   *
   * Constructs an ImageView from an internal object handle.
   *
   * \param handle Internal image view handle.
   *
   * \attention 1. \p handle must reference a valid ImageView implementation.
   */
  explicit ImageView(void *handle) noexcept : Object(handle) {}
};

} // namespace strobe::rhi

// complete fwd definition.
#include "strobe/rhi/objects/image.hpp"
