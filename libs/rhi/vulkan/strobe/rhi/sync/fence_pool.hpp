#pragma once

#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/sync/fence.hpp"
#include "strobe/rhi/sync/fence_callback_flag.hpp"

namespace strobe::rhi {

class FencePool : public Object<FencePool> {
  friend class Object<FencePool>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;

public:
  using Object::Object;
  explicit FencePool(void *handle) noexcept : Object(handle) {}

  Fence allocate(void *pUserData = nullptr,
                 void (*callback)(void *,
                                  FenceCallbackFlag) = nullptr) noexcept;
};

} // namespace strobe::rhi
