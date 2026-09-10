#pragma once

#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/sync/fence.hpp"
#include "strobe/rhi/sync/fence_callback_flag.hpp"

namespace strobe::rhi {

class FencePool : public Object<FencePool> {
public:
  explicit FencePool(void *handle) noexcept : Object(handle) {}
  FencePool() noexcept : Object(nullptr) {}
  FencePool(const FencePool &) noexcept;
  FencePool(FencePool &&) noexcept;
  FencePool &operator=(const FencePool &) noexcept;
  FencePool &operator=(FencePool &&) noexcept;
  ~FencePool() noexcept;

  Fence allocate(void *pUserData = nullptr,
                 void (*callback)(void *,
                                  FenceCallbackFlag) = nullptr) noexcept;
};

} // namespace strobe::rhi
