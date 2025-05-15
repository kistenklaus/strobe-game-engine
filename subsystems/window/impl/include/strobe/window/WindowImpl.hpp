#pragma once

#include <string_view>
#include <strobe/core/sync/spsc.hpp>
#include <strobe/lina.hpp>
#include <strobe/memory.hpp>

namespace strobe::window {

class WindowImpl {
 public:
  WindowImpl(uvec2 size, std::string_view title,
             PolyAllocatorReference allocator);
  ~WindowImpl();

  WindowImpl(const WindowImpl&) = delete;
  WindowImpl& operator=(const WindowImpl&) = delete;
  WindowImpl(WindowImpl&&) = delete;
  WindowImpl& operator=(WindowImpl&&) = delete;

  bool shouldClose() const;
  void close();

  void setSize(unsigned int widht, unsigned int height);
  uvec2 getSize() const;
  uvec2 getFramebufferSize() const;

  void setTitle(std::string_view title);
  std::string_view getTitle() const;

  void setResizable(bool resizable);
  bool isResizable() const;

  spsc::SharedReceiver<int> keyboardEventChannel();
  spsc::SharedReceiver<int> mouseEventChannel();

 private:
  void* m_internals;
};

}  // namespace strobe::window
