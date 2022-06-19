#pragma once
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "renderer/RendererBackendAPI.hpp"
#include "types/inttypes.hpp"
#include "window/Keyboard.hpp"
#include "window/Mouse.hpp"
#include "window/WindowBackendAPI.hpp"
#include "window/WindowSizeCallback.hpp"

namespace sge {

class WindowBackend {
 public:
  const RendererBackendAPI m_renderer_backend_api;

 protected:
  std::string m_title;
  std::unique_ptr<Mouse> m_mouse;
  std::unique_ptr<Keyboard> m_keyboard;
  std::vector<WindowSizeCallback *> m_window_size_callbacks;

 private:
  uint32_t m_width;
  uint32_t m_height;

 public:
  WindowBackend(const u32 width, const u32 height, const std::string title,
                const RendererBackendAPI renderer_backend_api,
                std::unique_ptr<Mouse> mouse,
                std::unique_ptr<Keyboard> keyboard);
  virtual ~WindowBackend() = default;
  virtual boolean shouldClose() = 0;
  virtual void swapBuffers() = 0;
  virtual void pollEvents() = 0;
  virtual void close() = 0;
  virtual void destroy() = 0;
  virtual void setWidth(u32 width) = 0;
  virtual void setHeight(u32 height) = 0;
  virtual void setSize(u32 width, u32 height) {
    setWidth(width);
    setHeight(height);
  }
  inline u32 getWidth() const { return m_width; }
  inline u32 getHeight() const { return m_height; }
  inline const std::string &getTitle() { return m_title; }
  inline Mouse &getMouse() { return *m_mouse; }
  inline Keyboard &getKeyboard() { return *m_keyboard; }
  inline void addWindowSizeCallback(WindowSizeCallback *windowSizeCallback) {
    this->m_window_size_callbacks.push_back(windowSizeCallback);
  }
  inline void removeWindowSizeCallback(WindowSizeCallback *windowSizeCallback) {
    this->m_window_size_callbacks.erase(
        std::find(this->m_window_size_callbacks.begin(),
                  this->m_window_size_callbacks.end(), windowSizeCallback));
  }
  void setWidthInternal(const u32 width);
  void setHeightInternal(const u32 height);
  void setSizeInternal(const u32 width, const u32 height,
                       const boolean envokeCallacks);
};

}  // namespace sge
