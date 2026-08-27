#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/vulkan/context/context.hpp"
#include <atomic>

namespace strobe::rhi {

struct ContextImpl {
  ContextImpl(const vulkan::ContextCreateInfo &info) noexcept
      : allocator{}, context{info, &allocator} {}

  ~ContextImpl() noexcept {
    if (pfnDelete) {
      pfnDelete(pUserData);
    }
  }

  std::atomic<uint64_t> refCount{1};
  strobe::rhi::allocator allocator;
  vulkan::Context context;
  void *pUserData = nullptr;
  void (*pfnDelete)(void *) = nullptr;
};

void pin_context(void *h) {
  assert(h);
  auto *impl = static_cast<ContextImpl *>(h);
  impl->refCount.fetch_add(1, std::memory_order_relaxed);
}

void unpin_context(void *h) {
  if (h == nullptr) {
    return;
  }
  auto *impl = static_cast<ContextImpl *>(h);
  if (impl->refCount.fetch_sub(1, std::memory_order_acq_rel) != 1) {
    return;
  }
  delete impl;
}

Context::Context(const vulkan::ContextCreateInfo &info) noexcept
    : m_handle(new ContextImpl(info)) {}

Context::Context(const Context &o) noexcept : m_handle(o.m_handle) {
  if (m_handle != nullptr) {
    pin_context(m_handle);
  }
}

Context::Context(Context &&o) noexcept
    : m_handle(std::exchange(o.m_handle, nullptr)) {}

Context &Context::operator=(const Context &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_context(o.m_handle);
  }
  unpin_context(m_handle);
  m_handle = o.m_handle;
  return *this;
}

Context &Context::operator=(Context &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_context(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

Context::~Context() noexcept { unpin_context(m_handle); }

vulkan::Context *Context::ctx() const noexcept {
  return &static_cast<ContextImpl *>(m_handle)->context;
}

strobe::rhi::allocator_ref Context::get_allocator() const noexcept {
  return &static_cast<ContextImpl *>(m_handle)->allocator;
}

void Context::delete_hook(void *pUserData,
                          void (*pfnDelete)(void *) noexcept) const noexcept {
  static_cast<ContextImpl *>(m_handle)->pUserData = pUserData;
  static_cast<ContextImpl *>(m_handle)->pfnDelete = pfnDelete;
}

} // namespace strobe::rhi
