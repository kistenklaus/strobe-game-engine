#include "strobe/rhi/sync/fence_impl.hpp"

#include "strobe/rhi/sync/fence_callback_flag.hpp"
#include "strobe/rhi/sync/fence_pool_impl.hpp"

strobe::rhi::FenceImpl::~FenceImpl() noexcept {
  auto *impl = object_handle_ptr<FencePoolImpl>(pool);
  if (node != nullptr) {
    if (callback) {
      callback(pUserData, FenceCallbackFlag::aborted);
    }
    // fence was not signaled, but still dropped,
    impl->recycle(node);
  }
}
