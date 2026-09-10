#include "strobe/rhi/objects/tlas.hpp"
#include "strobe/rhi/bvh/bvh_impl.hpp"
#include "strobe/rhi/handle.hpp"

namespace strobe::rhi {

void Tlas::pin(void *handle) noexcept { pin_void_handle<BvhImpl>(handle); }
void Tlas::unpin(void *handle) noexcept { unpin_void_handle<BvhImpl>(handle); }

} // namespace strobe::rhi
