#pragma once

#include <atomic>
#include <print>
#include <strobe/core/containers/small_vector.hpp>
#include <strobe/window/WindowHandle.hpp>
#include <type_traits>

#include "./Action.hpp"
#include "./Key.hpp"
#include "strobe/core/events/event_listener.hpp"
#include "strobe/core/memory/smart_pointers/BlockRef.hpp"
#include "strobe/input/allocator.hpp"
#include "strobe/window/KeyboardEvent.hpp"

namespace strobe {

class InputSystem;

class Keyboard;

namespace input::details {

class KeyboardControlBlock {
  using bitset_type = std::uint64_t;
  static constexpr std::size_t KEY_WORD_COUNT =
      (KEY_COUNT + (sizeof(bitset_type) * 8) - 1) / (sizeof(bitset_type) * 8);

 public:
  friend InputSystem;
  void addSource(const WindowHandle& window) {
    auto handler = window->addKeyboardEventListener(
        EventListenerRef<window::KeyboardEvent>::fromMemberFunction<
            KeyboardControlBlock, &KeyboardControlBlock::keyCallback>(this));
    m_sources.push_back(std::move(handler));
  }

  bool isKeyDown(Key key) const {
    using uint_key = std::underlying_type_t<Key>;
    uint_key keyidx = static_cast<uint_key>(key);
    assert(keyidx < KEY_COUNT);
    uint_key wordidx = keyidx / (sizeof(bitset_type) * 8);
    uint_key offset = keyidx % (sizeof(bitset_type) * 8);
    assert(wordidx < KEY_WORD_COUNT);
    return (m_keyStateBack[wordidx].load(std::memory_order_relaxed) &
            (bitset_type(0x1) << offset)) != 0;
  }

  explicit KeyboardControlBlock(allocator_ref allocator)
      : m_sources(allocator) {}

 private:
  void pollEvents() {
    for (std::size_t i = 0; i < KEY_WORD_COUNT; ++i) {
      bitset_type v = m_keyStateBack[i].load(std::memory_order_relaxed);
      m_keyState[i].store(v, std::memory_order_relaxed);
    }
  }

  void keyCallback(const strobe::window::KeyboardEvent& event) {
    using uint_key = std::underlying_type_t<Key>;
    uint_key keyidx = static_cast<uint_key>(event.payload().key);
    assert(keyidx < KEY_COUNT);
    uint_key wordidx = keyidx / (sizeof(bitset_type) * 8);
    assert(wordidx < KEY_WORD_COUNT);
    uint_key offset = keyidx % (sizeof(bitset_type) * 8);

    if (event.payload().action == Action::Press) {
      m_keyStateBack[wordidx].fetch_or(bitset_type(0x1) << offset,
                                       std::memory_order_relaxed);
    } else if (event.payload().action == Action::Release) {
      m_keyStateBack[wordidx].fetch_and(~(bitset_type(0x1) << offset),
                                        std::memory_order_relaxed);
    }
  }

  // THIS right here should probably be replaced with a SmallLockFreeStack.
  SmallVector<EventListenerHandle, input::allocator_ref, 1> m_sources;
  std::atomic<std::uint64_t> m_keyState[KEY_WORD_COUNT];
  std::atomic<std::uint64_t> m_keyStateBack[KEY_WORD_COUNT];
};

};  // namespace input::details

class Keyboard {
 public:
  friend class InputSystem;
  void addSource(const WindowHandle& window) {
    return m_controlBlock->addSource(window);
  }

  bool isKeyDown(Key key) const { return m_controlBlock->isKeyDown(key); }

 private:
  using Block = ::strobe::Block<input::details::KeyboardControlBlock,
                                input::allocator_ref>;
  using Ref = Block::Ref;

  Keyboard(Ref controlBlock) : m_controlBlock(std::move(controlBlock)) {}

  // using Block =

  Ref m_controlBlock;
};

}  // namespace strobe
