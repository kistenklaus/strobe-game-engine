#pragma once

#include "strobe/gpu/device/handle.hpp"
#include "strobe/gpu/vulkan/context/context.hpp"
namespace strobe::gpu {

struct Context {
  Context(const vulkan::ContextCreateInfo &createInfo)
      : m_handle(make_handle<vulkan::Context>(createInfo)) {}
  Context(const Context &o) noexcept : m_handle(o.m_handle) {
    if (m_handle != nullptr) {
      pin_handle(m_handle);
    }
  }
  Context(Context &&o) noexcept
      : m_handle(std::exchange(o.m_handle, nullptr)) {}
  Context &operator=(const Context &o) noexcept {
    if (this == &o) {
      return *this;
    }
    if (o.m_handle != nullptr) {
      pin_handle(o.m_handle);
    }
    unpin_handle(m_handle);
    m_handle = o.m_handle;
    return *this;
  }
  Context &operator=(Context &&o) noexcept {
    if (this == &o) {
      return *this;
    }
    unpin_handle(m_handle);
    m_handle = std::exchange(o.m_handle, nullptr);
    return *this;
  }
  ~Context() { unpin_handle(m_handle); }

  vulkan::Context *get() { return handle_ptr(m_handle); }
  vulkan::Context *get() const { return handle_ptr(m_handle); }

private:
  handle<vulkan::Context> m_handle;
};

} // namespace strobe::gpu
