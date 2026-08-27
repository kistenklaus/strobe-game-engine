#pragma once

#include "strobe/rhi/vulkan/context/context.hpp"
#include "strobe/rhi/vulkan/context/create_info.hpp"

namespace strobe::rhi {

class Context {
public:
  Context(const vulkan::ContextCreateInfo &createInfo) noexcept;
  Context(const Context &) noexcept;
  Context(Context &&) noexcept;
  Context &operator=(const Context &) noexcept;
  Context &operator=(Context &&) noexcept;
  ~Context() noexcept;

  vulkan::Context *ctx() const noexcept;
  strobe::rhi::allocator_ref get_allocator() const noexcept;

  void delete_hook(void *pUserData,
                   void (*pfnDelete)(void *) noexcept) const noexcept;

private:
  void *m_handle;
};

} // namespace strobe::rhi
