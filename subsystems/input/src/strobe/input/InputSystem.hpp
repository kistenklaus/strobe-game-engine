#pragma once

#include <mutex>

#include "strobe/core/containers/vector.hpp"
#include "strobe/core/memory/smart_pointers/BlockRef.hpp"
#include "strobe/input/Keyboard.hpp"
#include "strobe/input/Mouse.hpp"
#include "strobe/input/allocator.hpp"
#include "strobe/window/WindowManager.hpp"

namespace strobe {

class InputSystem {
 public:
  InputSystem([[maybe_unused]] const WindowManager* windowManager,
              const input::allocator& alloc = {});
  InputSystem([[maybe_unused]] const WindowManager* windowManager,
              input::allocator&& alloc);

  InputSystem(const InputSystem& o) = delete;
  InputSystem& operator=(const InputSystem& o) = delete;
  InputSystem(InputSystem&& o) = delete;
  InputSystem& operator=(InputSystem&& o) = delete;

  Keyboard createKeyboard();
  Mouse createMouse();
  void pollEvents();

 private:
  std::mutex m_mutex;
  Vector<Keyboard::Block> m_keyboards;
  Vector<Mouse::Block> m_mouses;
  input::allocator m_allocator;
};

}  // namespace strobe
