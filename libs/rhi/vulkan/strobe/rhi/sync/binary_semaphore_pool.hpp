#pragma once

#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/sync/binary_semaphore.hpp"
namespace strobe::rhi {

class BinarySemaphorePool : public Object<BinarySemaphorePool> {
  public:
    explicit BinarySemaphorePool(void* handle) noexcept : Object(handle) {}
    BinarySemaphorePool() noexcept : Object(nullptr) {}
    BinarySemaphorePool(const BinarySemaphorePool &) noexcept;
    BinarySemaphorePool(BinarySemaphorePool &&) noexcept;
    BinarySemaphorePool &operator=(const BinarySemaphorePool &) noexcept;
    BinarySemaphorePool &operator=(BinarySemaphorePool &&) noexcept;
    ~BinarySemaphorePool() noexcept;

    BinarySemaphore allocate() noexcept;
};

}
