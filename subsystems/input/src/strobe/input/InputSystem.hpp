#pragma once

#include <mutex>

#include "strobe/core/containers/vector.hpp"
#include "strobe/core/memory/smart_pointers/BlockRef.hpp"
#include "strobe/input/Keyboard.hpp"
#include "strobe/input/Mouse.hpp"
#include "strobe/input/allocator.hpp"
#include "strobe/window/WindowSubsystem.hpp"

namespace strobe {

class InputSystem {
 public:
  InputSystem([[maybe_unused]] const WindowManager* windowManager,
              const input::allocator& alloc = {})
      : m_allocator(alloc) {}
  InputSystem([[maybe_unused]] const WindowManager* windowManager,
              input::allocator&& alloc)
      : m_allocator(std::move(alloc)) {}

  InputSystem(const InputSystem& o) = delete;
  InputSystem& operator=(const InputSystem& o) = delete;
  InputSystem(InputSystem&& o) = delete;
  InputSystem& operator=(InputSystem&& o) = delete;

  Keyboard createKeyboard() {
    std::lock_guard lck{m_mutex};
    input::allocator_ref allocRef = &m_allocator;
    Keyboard::Block keyboard =
        makeBlock<input::details::KeyboardControlBlock>(allocRef, allocRef);
    auto ref = keyboard.ref();
    m_keyboards.push_back(std::move(keyboard));
    return Keyboard{ref};
  }
  Mouse createMouse() {
    std::lock_guard lck{m_mutex};
    input::allocator_ref allocRef = &m_allocator;
    Mouse::Block mouse =
        makeBlock<input::details::MouseControlBlock>(allocRef, allocRef);
    Mouse::Ref ref = mouse.ref();
    m_mouses.push_back(std::move(mouse));
    return Mouse{ref};
  }

  void pollEvents() {
    std::lock_guard lck{m_mutex};
    auto it = m_keyboards.begin();
    while (it != m_keyboards.end()) {
      if (it->isReferenced()) {
        (*it)->pollEvents();
        it++;
      } else {
        it = m_keyboards.erase(it);
      }
    }
  }

 private:
  std::mutex m_mutex;
  Vector<Keyboard::Block> m_keyboards;
  Vector<Mouse::Block> m_mouses;
  input::allocator m_allocator;
};

}  // namespace strobe
