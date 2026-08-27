#pragma once

#include "strobe/rhi/objects/object.hpp"

namespace strobe::rhi {

struct StagingPool : Object<StagingPool> {
public:
  explicit StagingPool(void *handle) noexcept : Object(handle) {}
  StagingPool() noexcept : Object(nullptr) {}
  StagingPool(const StagingPool &) noexcept;
  StagingPool(StagingPool &&) noexcept;
  StagingPool &operator=(const StagingPool &) noexcept;
  StagingPool &operator=(StagingPool &&) noexcept;
  ~StagingPool() noexcept;
};

} // namespace strobe::rhi
