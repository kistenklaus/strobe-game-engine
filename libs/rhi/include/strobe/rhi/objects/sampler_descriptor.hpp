#pragma once

#include "strobe/rhi/objects/object.hpp"
namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Sampler descriptor handle
 * Defined in header <strobe/rhi/rhi.hpp>
 * \code{.cpp}
 * class SamplerDescriptor : public Object<SamplerDescriptor>;
 *
 */
class SamplerDescriptor : public Object<SamplerDescriptor> {
  friend class Object<SamplerDescriptor>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;

public:
  using Object::Object;
};

} // namespace strobe::rhi
