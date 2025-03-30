#pragma once

#include <string_view>
#include <utility>
#include <vulkan/vulkan.hpp>

namespace strobe::window {

class WindowContext {
 public:
  WindowContext() = default;
  virtual ~WindowContext() = default;
  WindowContext(const WindowContext&) = delete;
  WindowContext& operator=(const WindowContext&) = delete;

  virtual bool shouldClose() = 0;
  virtual void pollEvents() = 0;
  virtual void close() = 0;
  virtual void setWidth(unsigned int width) = 0;
  virtual void setHeight(unsigned int height) = 0;
  virtual void setSize(unsigned int widht, unsigned int height) = 0;
  virtual unsigned int getWidth() = 0;
  virtual unsigned int getHeight() = 0;
  virtual std::pair<unsigned int, unsigned int> getSize() = 0;
  virtual void setTitle(std::string_view title) = 0;
  virtual std::string_view getTitle() = 0;
  virtual void setResizable(bool resizable) = 0;
  virtual bool isResizable() = 0;
  virtual vk::SurfaceKHR createSurface(vk::Instance instance) = 0;
  virtual std::span<const char* const> getRequiredVkInstanceExtensions() = 0;
};

}  // namespace strobe::window::api
