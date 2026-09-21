#include "strobe/rhi/objects/blas.hpp"
#include "strobe/rhi/bvh/bvh_impl.hpp"
#include "strobe/rhi/handle.hpp"

namespace strobe::rhi {

void Blas::pin(void *handle) noexcept { pin_void_handle<BvhImpl>(handle); }
void Blas::unpin(void *handle) noexcept { unpin_void_handle<BvhImpl>(handle); }

uint64_t Blas::address() const noexcept {
  if (m_handle == nullptr) {
    return 0;
  }
  auto *impl = void_handle_ptr<BvhImpl>(m_handle);
  return impl->address();
}

} // namespace strobe::rhi
