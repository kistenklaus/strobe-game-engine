#pragma once

#include <string_view>
#include <strobe/lina.hpp>
#include <strobe/memory.hpp>
#include <utility>

namespace strobe::window {

class WindowImpl {
 public:
  WindowImpl(uvec2 size, std::string_view title, PolyAllocator allocator);
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

 private:
  void* m_internals;
};

}  // namespace strobe::window::details
