#include "strobe/rhi/stage/staging_pool.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/stage/staging_pool_impl.hpp"

namespace strobe::rhi {

StagingPool::StagingPool(const StagingPool &o) noexcept : Object(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<StagingPoolImpl>(m_handle);
  }
}

StagingPool::StagingPool(StagingPool &&o) noexcept
    : Object(std::exchange(o.m_handle, nullptr)) {}

StagingPool &StagingPool::operator=(const StagingPool &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<StagingPoolImpl>(o.m_handle);
  }
  unpin_void_handle<StagingPoolImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

StagingPool &StagingPool::operator=(StagingPool &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<StagingPoolImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

StagingPool::~StagingPool() noexcept {
  unpin_void_handle<StagingPoolImpl>(m_handle);
}

} // namespace strobe::rhi
