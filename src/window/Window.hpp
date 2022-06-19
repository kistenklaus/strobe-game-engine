#pragma once
#include <memory>

#include "window/WindowBackend.hpp"
#include "window/WindowBackendAPI.hpp"

namespace sge {

class Window {
 public:
  Window(const u32 width, const u32 height, const std::string title,
         const RendererBackendAPI renderer_backend_api,
         const WindowBackendAPI window_api);
  ~Window();

  boolean shouldClose();
  void close();
  void swapBuffers();
  void pollEvents();
  void dispose();
  WindowBackendAPI getBackendAPI() const;
  const WindowBackend& getBackend() const;
  const u32 getWidth() const;
  const u32 getHeight() const;

 private:
  std::unique_ptr<WindowBackend> m_backend;
  WindowBackendAPI m_window_backend_api;
};

}  // namespace sge
