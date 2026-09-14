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
   * \code{.cpp}
   * Format format() const noexcept;
   * \endcode
   *
   * Returns the format used by this image view.
   *
   * \return Image view format.
   */
  Format format() const noexcept;

  /**
   * \brief Gets source image.
   * \code{.cpp}
   * const Image& image() const noexcept;
   * \endcode
   *
   * Returns the image referenced by this view.
   *
   * \return Referenced image.
   */
  const Image &image() const noexcept;
};

} // namespace strobe::rhi

// complete fwd definition.
#include "strobe/rhi/objects/image.hpp"
