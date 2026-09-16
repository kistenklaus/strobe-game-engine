#pragma once

#include "strobe/rhi/objects/object.hpp"
namespace strobe::rhi {

class SamplerDescriptorArray : public Object<SamplerDescriptorArray> {
  friend class Object<SamplerDescriptorArray>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;

public:
  using Object::Object;
};

} // namespace strobe::rhi
