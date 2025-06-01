#include "strobe/window/WindowContext.hpp"

#include "GlfwWindow.hpp"
#include "GlfwWindowContext.hpp"
#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/PolyAllocator.hpp"

namespace strobe::window {

WindowContext::WindowContext(PolyAllocatorReference allocator) {
  using ATraits = AllocatorTraits<PolyAllocatorReference>;
  auto ctx = ATraits::allocate<GlfwWindowContext>(allocator);
  new (ctx) GlfwWindowContext(std::move(allocator));
  m_internals = static_cast<void*>(ctx);
}

WindowContext::~WindowContext() {
  if (m_internals != nullptr) {
    auto ctx = static_cast<GlfwWindowContext*>(m_internals);
    auto allocator = ctx->getAllocator();
    ctx->~GlfwWindowContext();
    using ATraits = AllocatorTraits<decltype(allocator)>;
    ATraits::deallocate<GlfwWindowContext>(allocator, ctx);
    m_internals = nullptr;
  }
}

Window WindowContext::createWindow(uvec2 size, std::string_view title,
                                   bool resizable, bool closeOnCallback) {
  auto ctx = static_cast<GlfwWindowContext*>(m_internals);
  auto allocator = ctx->getAllocator();
  using ATraits = AllocatorTraits<decltype(allocator)>;
  GlfwWindow* win = ATraits::allocate<GlfwWindow>(allocator);
  new (win) GlfwWindow(ctx, size, title, resizable, closeOnCallback);
  return Window(static_cast<void*>(win));
}

};  // namespace strobe::window
