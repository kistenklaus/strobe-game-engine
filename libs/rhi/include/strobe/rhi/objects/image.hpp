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
 * \brief todo
 */
class Image : public Object<Image> {
  friend class Object<Image>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;

public:
  using Object::Object;
  ImageType type() const noexcept;
  Format format() const noexcept;
  uvec3 extent() const noexcept;
  uint32_t mip_levels() const noexcept;
  uint32_t arrayLayers() const noexcept;
  SampleCount samples() const noexcept;
};

} // namespace strobe::rhi
