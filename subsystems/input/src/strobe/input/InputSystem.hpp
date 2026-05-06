#pragma once

#include "strobe/core/containers/small_vector.hpp"
#include "strobe/core/containers/vector.hpp"
#include "strobe/input/Keyboard.hpp"
#include "strobe/input/Mouse.hpp"
#include "strobe/input/allocator.hpp"
#include "strobe/window/WindowManager.hpp"

namespace strobe {

class InputSystem {
public:
  InputSystem(const WindowManager *windowManager,
              const input::allocator &alloc = {});
  InputSystem(const WindowManager *windowManager, input::allocator &&alloc);

  InputSystem(const InputSystem &o) = delete;
  InputSystem &operator=(const InputSystem &o) = delete;
  InputSystem(InputSystem &&o) = delete;
  InputSystem &operator=(InputSystem &&o) = delete;

  // fully thread safe.
  Keyboard createKeyboard();
  // fully thread safe.
  Mouse createMouse();

  // must not be called concurrently.
  void pollEvents();

private:
  SmallVector<Keyboard::Block, 2> m_newKeyboards;
  SmallVector<Mouse::Block, 2> m_newMouse;
  Vector<Keyboard::Block> m_keyboards;
  Vector<Mouse::Block> m_mouses;
  input::allocator m_allocator;
};

} // namespace strobe
