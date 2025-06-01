#include "./InputSystem.hpp"

namespace strobe {

InputSystem::InputSystem([[maybe_unused]] const WindowManager* windowManager,
                         const input::allocator& alloc)
    : m_allocator(alloc) {}

InputSystem::InputSystem([[maybe_unused]] const WindowManager* windowManager,
                         input::allocator&& alloc)
    : m_allocator(std::move(alloc)) {}

Keyboard InputSystem::createKeyboard() {
  std::lock_guard lck{m_mutex};
  input::allocator_ref allocRef = &m_allocator;
  Keyboard::Block keyboard =
      makeBlock<input::details::KeyboardControlBlock>(allocRef, allocRef);
  auto ref = keyboard.ref();
  m_keyboards.push_back(std::move(keyboard));
  return Keyboard{ref};
}

Mouse InputSystem::createMouse() {
  std::lock_guard lck{m_mutex};
  input::allocator_ref allocRef = &m_allocator;
  Mouse::Block mouse =
      makeBlock<input::details::MouseControlBlock>(allocRef, allocRef);
  Mouse::Ref ref = mouse.ref();
  m_mouses.push_back(std::move(mouse));
  return Mouse{ref};
}

void InputSystem::pollEvents() {
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
}  // namespace strobe
