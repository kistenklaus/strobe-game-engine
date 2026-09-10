#pragma once

#include "strobe/rhi/objects/object.hpp"

namespace strobe::rhi {

struct StagingPool : public Object<StagingPool> {
  friend class Object<StagingPool>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;

public:
  using Object::Object;
  explicit StagingPool(void *handle) noexcept : Object(handle) {}
};

} // namespace strobe::rhi
