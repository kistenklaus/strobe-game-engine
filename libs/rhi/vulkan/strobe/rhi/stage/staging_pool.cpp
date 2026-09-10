#include "strobe/rhi/stage/staging_pool.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/stage/staging_pool_impl.hpp"

namespace strobe::rhi {

void StagingPool::pin(void *handle) noexcept {
  pin_void_handle<StagingPoolImpl>(handle);
}

void StagingPool::unpin(void *handle) noexcept {
  unpin_void_handle<StagingPoolImpl>(handle);
}

} // namespace strobe::rhi
