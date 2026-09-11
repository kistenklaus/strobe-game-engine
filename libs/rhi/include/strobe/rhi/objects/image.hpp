#pragma once

#include "strobe/core/lina/vec.hpp"
#include "strobe/rhi/objects/image_view.hpp"
#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/types/format.hpp"
#include "strobe/rhi/types/image_type.hpp"
#include "strobe/rhi/types/sample_count.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Device image object.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \code{.cpp}
 * class Image : public Object<Image>;
 * \endcode
 *
 * Represents a device-owned image and its immutable creation properties.
 */
class Image : public Object<Image> {
  friend class Object<Image>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;

public:
  using Object::Object;

  /**
   * \brief Gets image type.
   *
   * Returns the dimensionality of the image.
   *
   * \return Image type.
   */
  ImageType type() const noexcept;

  /**
   * \brief Gets image format.
   *
   * Returns the image format.
   *
   * \return Image format.
   */
  Format format() const noexcept;

  /**
   * \brief Gets image extent.
   *
   * Returns the image extent in texels.
   *
   * \return Image extent.
   */
  uvec3 extent() const noexcept;

  /**
   * \brief Gets mip levels.
   *
   * Returns the number of mip levels.
   *
   * \return Number of mip levels.
   */
  uint32_t mip_levels() const noexcept;

  /**
   * \brief Gets array layers.
   *
   * Returns the number of array layers.
   *
   * \return Number of array layers.
   */
  uint32_t arrayLayers() const noexcept;

  /**
   * \brief Gets sample count.
   *
   * Returns the image sample count.
   *
   * \return Sample count.
   */
  SampleCount samples() const noexcept;
};

} // namespace strobe::rhi
