#include "strobe/rhi/sync/fence_impl.hpp"

#include "strobe/rhi/sync/fence_pool_impl.hpp"

strobe::rhi::FenceImpl::~FenceImpl() noexcept {
  auto *impl = object_handle_ptr<FencePoolImpl>(pool);
  if (node != nullptr) {
    // techniqually a weird semantic, but it's for 
    // present fences this somehow works.
    if (callback) {
      callback(pUserData);
    }
    // fence was not signaled, but still dropped,
    // -> don't call the callback
    impl->recycle(node);
  }
}
