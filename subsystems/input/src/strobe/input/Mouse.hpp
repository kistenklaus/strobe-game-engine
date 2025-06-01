#pragma once

#include <strobe/core/containers/small_vector.hpp>
#include <strobe/core/events/event_listener_handle.hpp>
#include <strobe/core/memory/smart_pointers/BlockRef.hpp>
#include <strobe/window/MouseScrollEvent.hpp>

#include "strobe/input/MouseButton.hpp"
#include "strobe/input/allocator.hpp"
#include "strobe/input/details/MouseControlBlock.hpp"
#include "strobe/window/WindowHandle.hpp"

namespace strobe {

class Mouse {
 public:
  using Block =
      ::strobe::Block<input::details::MouseControlBlock, input::allocator_ref>;
  using Ref = Block::Ref;
  friend class strobe::InputSystem;

  void addSource(const WindowHandle& window);
  vec2 pos() const;
  vec2 scroll() const;
  bool isButtonDown(MouseButton button) const;

 private:
  Mouse(Ref controlBlock) : m_controlBlock(std::move(controlBlock)) {}
  Ref m_controlBlock;
};

}  // namespace strobe
