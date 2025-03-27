#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <semaphore>
#include <string>

#include "input/Keyboard.hpp"
#include "window/ClientApi.hpp"
#include "window/WindowContext.hpp"
namespace strobe::window::details {

struct WindowControlBlock {
  std::mutex mutex;
  // locked resources
  unsigned int width = 600;
  unsigned int height = 400;
  bool resizable = false;
  bool visible = true;
  bool vsync = false;
  std::string title;
  std::weak_ptr<WindowContext> context;

  // initalization
  ClientApi clientApi;
  std::condition_variable initalized;

  // input
  std::shared_ptr<input::Keyboard> keyboard;

};

}  // namespace strobe::window::details
