#include "strobe/rhi/device/context.hpp"
#include "strobe/core/memory/Mallocator.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/vulkan/context/context.hpp"

namespace strobe::rhi {

struct ContextImpl {
  ContextImpl(const vulkan::ContextCreateInfo &info) noexcept
      : allocator{}, context(info, &allocator) {}

  strobe::rhi::allocator allocator;
  vulkan::Context context;
};

Context::Context(const vulkan::ContextCreateInfo &createInfo) noexcept
    : m_handle(alloc_void_handle<ContextImpl, strobe::Mallocator>(
          strobe::Mallocator{}, createInfo)) {}

Context::Context(const Context &o) noexcept : m_handle(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<ContextImpl, strobe::Mallocator>(m_handle);
  }
}

Context::Context(Context &&o) noexcept
    : m_handle(std::exchange(o.m_handle, nullptr)) {}

Context &Context::operator=(const Context &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<ContextImpl, strobe::Mallocator>(o.m_handle);
  }
  unpin_void_handle<ContextImpl, strobe::Mallocator>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

Context &Context::operator=(Context &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<ContextImpl, strobe::Mallocator>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

Context::~Context() noexcept {
  unpin_void_handle<ContextImpl, strobe::Mallocator>(m_handle);
}

vulkan::Context *Context::ctx() const noexcept {
  return &void_handle_ptr<ContextImpl, strobe::Mallocator>(m_handle)->context;
}

strobe::rhi::allocator_ref Context::get_allocator() const noexcept {
  return &void_handle_ptr<ContextImpl, strobe::Mallocator>(m_handle)->allocator;
}

} // namespace strobe::rhi
