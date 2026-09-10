#pragma once

#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/sync/binary_semaphore.hpp"
namespace strobe::rhi {

class BinarySemaphorePool : public Object<BinarySemaphorePool> {
  friend class Object<BinarySemaphorePool>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;

public:
  using Object::Object;
  explicit BinarySemaphorePool(void *handle) noexcept : Object(handle) {}
  BinarySemaphore allocate() noexcept;
};

} // namespace strobe::rhi
