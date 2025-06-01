#pragma once

#include <atomic>
#include <strobe/core/containers/small_vector.hpp>
#include <strobe/window/WindowHandle.hpp>

#include "./Key.hpp"
#include "strobe/core/memory/smart_pointers/BlockRef.hpp"
#include "strobe/input/allocator.hpp"
#include "strobe/window/KeyboardEvent.hpp"
#include "strobe/input/details/KeyboardControlBlock.hpp"

namespace strobe {

class InputSystem;
class Keyboard;

class Keyboard {
 public:
  friend class InputSystem;
  void addSource(const WindowHandle& window);
  bool isKeyDown(Key key) const;

 private:
  using Block = ::strobe::Block<input::details::KeyboardControlBlock,
                                input::allocator_ref>;
  using Ref = Block::Ref;

  Keyboard(Ref controlBlock) : m_controlBlock(std::move(controlBlock)) {}

  Ref m_controlBlock;
};

}  // namespace strobe
