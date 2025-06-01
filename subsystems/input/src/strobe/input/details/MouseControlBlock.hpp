#pragma once

#include <cstdint>

#include "strobe/core/containers/small_vector.hpp"
#include "strobe/input/MouseButton.hpp"
#include "strobe/input/allocator.hpp"
#include "strobe/window/WindowHandle.hpp"
namespace strobe {
class InputSystem;
class Mouse;

}  // namespace strobe

namespace strobe::input::details {

class MouseControlBlock {
 public:
  explicit MouseControlBlock(allocator_ref alloc);

 private:
  friend ::strobe::InputSystem;
  friend ::strobe::Mouse;

  using bitset_type = std::uint64_t;
  static constexpr std::size_t BIT_COUNT = sizeof(bitset_type) * 8;

  static constexpr std::size_t BUTTON_BITSET_WORDS =
      (MOUSE_BUTTON_COUNT + BIT_COUNT - 1) / BIT_COUNT;

  void addSource(const WindowHandle& window);
  void mousePosCallback(const window::MouseMoveEvent& event);

  void mouseButtonCallback(const window::MouseButtonEvent& event);

  void mouseScrollCallback(const window::MouseScrollEvent& event);

  void pollEvents();

  bool isButtonDown(MouseButton button) const;

  SmallVector<EventListenerHandle, allocator_ref> m_sources;
  struct MousePos {
    std::atomic<float> x;
    std::atomic<float> y;
  };
  MousePos m_pos;
  MousePos m_posBack;
  struct ScrollDx {
    std::atomic<float> dx;
    std::atomic<float> dy;
  };
  ScrollDx m_scroll;
  ScrollDx m_scrollBack;
  std::atomic<bitset_type> m_mouseButtonState[BUTTON_BITSET_WORDS];
  std::atomic<bitset_type> m_mouseButtonStateBack[BUTTON_BITSET_WORDS];
};
}  // namespace strobe::input::details
