#include "./Keyboard.hpp"
#include "./Action.hpp"

namespace strobe {

namespace input::details {

void KeyboardControlBlock::addSource(const WindowHandle& window) {
  auto handler = window->addKeyboardEventListener(
      EventListenerRef<window::KeyboardEvent>::fromMemberFunction<
          KeyboardControlBlock, &KeyboardControlBlock::keyCallback>(this));
  m_sources.push_back(std::move(handler));
}

bool KeyboardControlBlock::isKeyDown(Key key) const {
  using uint_key = std::underlying_type_t<Key>;
  uint_key keyidx = static_cast<uint_key>(key);
  assert(keyidx < KEY_COUNT);
  uint_key wordidx = keyidx / (sizeof(bitset_type) * 8);
  uint_key offset = keyidx % (sizeof(bitset_type) * 8);
  assert(wordidx < KEY_WORD_COUNT);
  return (m_keyStateBack[wordidx].load(std::memory_order_relaxed) &
          (bitset_type(0x1) << offset)) != 0;
}

void KeyboardControlBlock::pollEvents() {
  for (std::size_t i = 0; i < KEY_WORD_COUNT; ++i) {
    bitset_type v = m_keyStateBack[i].load(std::memory_order_relaxed);
    m_keyState[i].store(v, std::memory_order_relaxed);
  }
}

void KeyboardControlBlock::keyCallback(
    const strobe::window::KeyboardEvent& event) {
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

}  // namespace input::details
void Keyboard::addSource(const WindowHandle& window) {
  return m_controlBlock->addSource(window);
}
bool Keyboard::isKeyDown(Key key) const {
  return m_controlBlock->isKeyDown(key);
}
}  // namespace strobe
