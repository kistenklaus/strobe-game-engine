#pragma once

#include "strobe/gpu/device/handle.hpp"
#include "strobe/gpu/device/profiler.hpp"
#include "strobe/gpu/vulkan/context/context.hpp"
//
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
//
#include <tracy/TracyVulkan.hpp>

namespace strobe::gpu {

struct Context {
  Context(const vulkan::ContextCreateInfo &createInfo)
      : m_handle(make_handle<Internal>(createInfo)) {}

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

  vulkan::Context *get() { return &handle_ptr(m_handle)->context; }
  vulkan::Context *get() const { return &handle_ptr(m_handle)->context; }

  profiler::Context *profiler() const noexcept {
    return &handle_ptr(m_handle)->profiler;
  }

private:
  struct Impl;
  struct Internal {
    Internal(const vulkan::ContextCreateInfo &info)
        : context(info), profiler(&context) {}
    ~Internal() noexcept {}
    vulkan::Context context;
    profiler::Context profiler;
  };
  handle<Internal> m_handle;
};

} // namespace strobe::gpu
