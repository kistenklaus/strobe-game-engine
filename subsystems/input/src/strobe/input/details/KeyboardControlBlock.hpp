#pragma once

#include "strobe/core/containers/small_vector.hpp"
#include "strobe/input/Key.hpp"
#include "strobe/input/allocator.hpp"
#include "strobe/window/WindowHandle.hpp"

namespace strobe {
class InputSystem;
};

namespace strobe::input::details {

class KeyboardControlBlock {
  using bitset_type = std::uint64_t;
  static constexpr std::size_t KEY_WORD_COUNT =
      (KEY_COUNT + (sizeof(bitset_type) * 8) - 1) / (sizeof(bitset_type) * 8);

 public:
  friend InputSystem;
  void addSource(const WindowHandle& window);

  bool isKeyDown(Key key) const;

  explicit KeyboardControlBlock(allocator_ref allocator)
      : m_sources(allocator) {}

 private:
  void pollEvents();

  void keyCallback(const strobe::window::KeyboardEvent& event);

  // THIS right here should probably be replaced with a SmallLockFreeStack.
  SmallVector<EventListenerHandle, input::allocator_ref, 1> m_sources;
  std::atomic<std::uint64_t> m_keyState[KEY_WORD_COUNT];
  std::atomic<std::uint64_t> m_keyStateBack[KEY_WORD_COUNT];
};

}  // namespace strobe::input::details
