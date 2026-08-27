#include "strobe/rhi/objects/fence.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/sync/fence_impl.hpp"
#include "strobe/rhi/vulkan/fence.hpp"

namespace strobe::rhi {

Fence::Fence(const Fence &o) noexcept : Object(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<FenceImpl>(m_handle);
  }
}

Fence::Fence(Fence &&o) noexcept : Object(std::exchange(o.m_handle, nullptr)) {}

Fence &Fence::operator=(const Fence &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<FenceImpl>(o.m_handle);
  }
  unpin_void_handle<FenceImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

Fence &Fence::operator=(Fence &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<FenceImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

Fence::~Fence() noexcept { unpin_void_handle<FenceImpl>(m_handle); }

bool Fence::wait(uint64_t timeout) const noexcept {
  ZoneScopedN("Fence::wait");
  auto *impl = void_handle_ptr<FenceImpl>(m_handle);
  return vulkan::wait_for_fence(impl->context.ctx(), impl->fence, timeout);
}

void Fence::reset() noexcept {
  ZoneScopedN("Fence::reset");
  auto *impl = void_handle_ptr<FenceImpl>(m_handle);
  vulkan::reset_fence(impl->context.ctx(), impl->fence);
}

bool Fence::wait_and_reset(uint64_t timeout) const noexcept {
  ZoneScopedN("Fence::wait_and_reset");
  auto *impl = void_handle_ptr<FenceImpl>(m_handle);
  bool timedout =
      vulkan::wait_for_fence(impl->context.ctx(), impl->fence, timeout);
  if (!timedout) {
    return false;
  }
  vulkan::reset_fence(impl->context.ctx(), impl->fence);
  return true;
}

bool Fence::signaled() const noexcept {
  ZoneScopedN("Fence::signaled");
  auto *impl = void_handle_ptr<FenceImpl>(m_handle);
  return vulkan::is_fence_signaled(impl->context.ctx(), impl->fence);
};

} // namespace strobe::rhi
