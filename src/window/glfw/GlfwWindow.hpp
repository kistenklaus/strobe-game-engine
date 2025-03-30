#pragma once

#include <memory>

#include "window/ClientApi.hpp"
#include "window/WindowContext.hpp"
#include "window/WindowControlBlock.hpp"

namespace strobe::window {

class GlfwWindow final : public WindowContext {
 public:
  GlfwWindow(ClientApi api,
             const std::shared_ptr<details::WindowControlBlock>& window);
  ~GlfwWindow();
  GlfwWindow(const GlfwWindow&) = delete;
  GlfwWindow& operator=(const GlfwWindow&) = delete;
  GlfwWindow(GlfwWindow&&) = delete;
  GlfwWindow& operator=(GlfwWindow&&) = delete;

  bool shouldClose() final override;
  void pollEvents() final override;
  void close() final override;
  void setWidth(unsigned int width) final override;
  void setHeight(unsigned int height) final override;
  void setSize(unsigned int widht, unsigned int height) final override;
  unsigned int getWidth() final override;
  unsigned int getHeight() final override;
  std::pair<unsigned int, unsigned int> getSize() final override;
  void setTitle(std::string_view title) final override;
  std::string_view getTitle() final override;
  void setResizable(bool resizable) final override;
  bool isResizable() final override;

  vk::SurfaceKHR createSurface(vk::Instance instance) final override;

  std::span<const char* const> getRequiredVkInstanceExtensions() final override;

 private:
  void* m_ptr;
};
};  // namespace strobe::window
