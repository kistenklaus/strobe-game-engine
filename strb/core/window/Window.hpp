#pragma once
#include "strb/core/window/Keyboard.hpp"
#include "strb/core/window/Mouse.hpp"
#include "strb/core/window/WindowSizeCallback.hpp"
#include "strb/stl.hpp"

namespace strb::window {

enum ClientApi { RENDER_API_VULKAN };
enum WindowApi { WINDOW_API_GLFW };

class Window {
 public:
  const ClientApi clientApi;
  const WindowApi windowApi;

 protected:
  strb::string title;
  Mouse *mouse;
  Keyboard *keyboard;
  strb::vector<WindowSizeCallback *> windowSizeCallbacks;

 private:
  uint32_t width;
  uint32_t height;

 public:
  Window(const uint32_t width, const uint32_t height, const strb::string title,
         const ClientApi clientApi, const WindowApi windowApi, Mouse *mouse,
         Keyboard *keyboard);
  virtual ~Window() {
    delete mouse;
    delete keyboard;
  }
  virtual strb::boolean shouldClose() = 0;
  virtual void swapBuffers() = 0;
  virtual void pollEvents() = 0;
  virtual void close() = 0;
  virtual void destroy() = 0;
  virtual void setWidth(uint32_t width) = 0;
  virtual void setHeight(uint32_t height) = 0;
  virtual void setSize(uint32_t width, uint32_t height) {
    setWidth(width);
    setHeight(height);
  }
  inline uint32_t getWidth() const { return width; }
  inline uint32_t getHeight() const { return height; }
  inline const std::string &getTitle() { return title; }
  inline Mouse &getMouse() { return *mouse; }
  inline Keyboard &getKeyboard() { return *keyboard; }
  inline void addWindowSizeCallback(WindowSizeCallback *windowSizeCallback) {
    this->windowSizeCallbacks.push_back(windowSizeCallback);
  }
  inline void removeWindowSizeCallback(WindowSizeCallback *windowSizeCallback) {
    this->windowSizeCallbacks.erase(std::find(this->windowSizeCallbacks.begin(),
                                              this->windowSizeCallbacks.end(),
                                              windowSizeCallback));
  }
  void setWidthInternal(const uint32_t width);
  void setHeightInternal(const uint32_t height);
  void setSizeInternal(const uint32_t width, const uint32_t height,
                       const strb::boolean envokeCallacks);
};

}  // namespace strb::window
