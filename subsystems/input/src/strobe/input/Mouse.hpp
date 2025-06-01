#pragma once

#include <atomic>
#include <type_traits>

#include "./allocator.hpp"
#include "strobe/core/containers/small_vector.hpp"
#include "strobe/core/events/event_listener_handle.hpp"
#include "strobe/core/memory/smart_pointers/BlockRef.hpp"
#include "strobe/input/Action.hpp"
#include "strobe/input/MouseButton.hpp"
#include "strobe/window/MouseScrollEvent.hpp"
#include "strobe/window/WindowHandle.hpp"

namespace strobe {

class InputSystem;
class Mouse;

namespace input::details {

class MouseControlBlock {
 public:
  explicit MouseControlBlock(allocator_ref alloc)
      : m_sources(std::move(alloc)) {}

 private:
  friend ::strobe::InputSystem;
  friend ::strobe::Mouse;

  using bitset_type = std::uint64_t;
  static constexpr std::size_t BIT_COUNT = sizeof(bitset_type) * 8;

  static constexpr std::size_t BUTTON_BITSET_WORDS =
      (MOUSE_BUTTON_COUNT + BIT_COUNT - 1) / BIT_COUNT;

  void addSource(const WindowHandle& window) {
    // register mouse move listener.
    {
      auto handler = window->addMouseMoveEventListener(
          EventListenerRef<window::MouseMoveEvent>::fromMemberFunction<
              MouseControlBlock, &MouseControlBlock::mousePosCallback>(this));
      m_sources.push_back(std::move(handler));
    }

    // register mouse button listener.
    {
      auto handler = window->addMouseButtonEventListener(
          EventListenerRef<window::MouseButtonEvent>::fromMemberFunction<
              MouseControlBlock, &MouseControlBlock::mouseButtonCallback>(
              this));
      m_sources.push_back(std::move(handler));
    }
  }
  void mousePosCallback(const window::MouseMoveEvent& event) {
    m_posBack.x.store(event.payload().x(), std::memory_order_relaxed);
    m_posBack.y.store(event.payload().y(), std::memory_order_relaxed);
  }

  void mouseButtonCallback(const window::MouseButtonEvent& event) {
    using uint_button = std::underlying_type_t<MouseButton>;
    uint_button bidx = static_cast<uint_button>(event.payload().button);
    assert(bidx < MOUSE_BUTTON_COUNT);
    uint_button wordidx = bidx / BIT_COUNT;
    assert(wordidx < BUTTON_BITSET_WORDS);
    uint_button offset = bidx % BIT_COUNT;
    if (event.payload().action == Action::Press) {
      m_mouseButtonStateBack[wordidx].fetch_or(bitset_type(0x1) << offset,
                                               std::memory_order_relaxed);
    } else if (event.payload().action == Action::Release) {
      m_mouseButtonStateBack[wordidx].fetch_and(~(bitset_type(0x1) << offset),
                                                std::memory_order_relaxed);
    }
  }

  void mouseScrollCallback(const window::MouseScrollEvent& event) {
    m_scrollBack.dx.fetch_add(event.payload().x());
    m_scrollBack.dy.fetch_add(event.payload().y());
  }

  void pollEvents() {
    float x = m_posBack.x.load(std::memory_order_relaxed);
    float y = m_posBack.y.load(std::memory_order_relaxed);
    m_pos.x.store(x, std::memory_order_relaxed);
    m_pos.y.store(y, std::memory_order_relaxed);

    float dx = m_scrollBack.dx.load(std::memory_order_relaxed);
    float dy = m_scrollBack.dy.load(std::memory_order_relaxed);
    m_scroll.dx.store(dx, std::memory_order_relaxed);
    m_scroll.dy.store(dy, std::memory_order_relaxed);
    m_scrollBack.dx.fetch_sub(dx);
    m_scrollBack.dy.fetch_sub(dy);

    for (std::size_t i = 0; i < BUTTON_BITSET_WORDS; ++i) {
      const bitset_type v =
          m_mouseButtonStateBack[i].load(std::memory_order_relaxed);
      m_mouseButtonState[i].store(v, std::memory_order_relaxed);
    }
  }

  bool isButtonDown(MouseButton button) const {
    using uint_button = std::underlying_type_t<MouseButton>;
    uint_button bidx = static_cast<uint_button>(button);
    assert(bidx < MOUSE_BUTTON_COUNT);
    uint_button wordidx = bidx / BIT_COUNT;
    assert(wordidx < BUTTON_BITSET_WORDS);
    uint_button offset = bidx % BIT_COUNT;
    return (m_mouseButtonState[wordidx].load(std::memory_order_relaxed) &
            (bitset_type(0x1) << offset)) != 0;
  }

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

}  // namespace input::details

class Mouse {
 public:
  using Block =
      ::strobe::Block<input::details::MouseControlBlock, input::allocator_ref>;
  using Ref = Block::Ref;
  friend class strobe::InputSystem;

  void addSource(const WindowHandle& window) {
    m_controlBlock->addSource(window);
  }

  vec2 pos() const {
    return vec2(m_controlBlock->m_pos.x.load(std::memory_order_relaxed),
                m_controlBlock->m_pos.y.load(std::memory_order_relaxed));
  }

  vec2 scroll() const {
    return vec2(m_controlBlock->m_scroll.dx.load(std::memory_order_relaxed),
                m_controlBlock->m_scroll.dy.load(std::memory_order_relaxed));
  }

  bool isButtonDown(MouseButton button) const {
    return m_controlBlock->isButtonDown(button);
  }

 private:
  Mouse(Ref controlBlock) : m_controlBlock(std::move(controlBlock)) {}
  Ref m_controlBlock;
};

}  // namespace strobe
