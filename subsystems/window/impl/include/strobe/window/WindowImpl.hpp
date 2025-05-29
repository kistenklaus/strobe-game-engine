#pragma once

#include <string_view>
#include <strobe/core/sync/spsc.hpp>
#include <strobe/lina.hpp>
#include <strobe/memory.hpp>

#include "strobe/core/events/event_listener.hpp"
#include "strobe/core/events/event_listener_handle.hpp"

namespace strobe::window {

class ResizeEvent {
 public:
  using payload_type = uvec2;

  explicit ResizeEvent(payload_type&& v) : m_payload(std::move(v)) {}
  explicit ResizeEvent(const payload_type& v) : m_payload(v) {}

  [[no_unique_address]] payload_type m_payload;

  const payload_type& payload() const { return m_payload; }

  constexpr bool canceled() const { return false; }
};

enum class MouseButton : uint8_t {
  Left = 0,
  Right = 1,
  Middle = 2,
  Extra1 = 3,
  Extra2 = 4,
  Extra3 = 5,
  Extra4 = 6,
  Extra5 = 7,
};

enum class Key : uint16_t {
  Space = 32,
  Apostrophe = 39, /* ' */
  Comma = 44,      /* , */
  Minus = 45,      /* - */
  Period = 46,     /* . */
  Slash = 47,      /* / */

  Key0 = 48,
  Key1 = 49,
  Key2 = 50,
  Key3 = 51,
  Key4 = 52,
  Key5 = 53,
  Key6 = 54,
  Key7 = 55,
  Key8 = 56,
  Key9 = 57,

  Semicolon = 59, /* ; */
  Equal = 61,     /* = */

  A = 65,
  B = 66,
  C = 67,
  D = 68,
  E = 69,
  F = 70,
  G = 71,
  H = 72,
  I = 73,
  J = 74,
  K = 75,
  L = 76,
  M = 77,
  N = 78,
  O = 79,
  P = 80,
  Q = 81,
  R = 82,
  S = 83,
  T = 84,
  U = 85,
  V = 86,
  W = 87,
  X = 88,
  Y = 89,
  Z = 90,

  LeftBracket = 91,  /* [ */
  Backslash = 92,    /* \ */
  RightBracket = 93, /* ] */
  GraveAccent = 96,  /* ` */

  Escape = 256,
  Enter = 257,
  Tab = 258,
  Backspace = 259,
  Insert = 260,
  Delete = 261,
  Right = 262,
  Left = 263,
  Down = 264,
  Up = 265,
  PageUp = 266,
  PageDown = 267,
  Home = 268,
  End = 269,

  CapsLock = 280,
  ScrollLock = 281,
  NumLock = 282,
  PrintScreen = 283,
  Pause = 284,

  F1 = 290,
  F2 = 291,
  F3 = 292,
  F4 = 293,
  F5 = 294,
  F6 = 295,
  F7 = 296,
  F8 = 297,
  F9 = 298,
  F10 = 299,
  F11 = 300,
  F12 = 301,

  LeftShift = 340,
  LeftControl = 341,
  LeftAlt = 342,
  LeftSuper = 343,
  RightShift = 344,
  RightControl = 345,
  RightAlt = 346,
  RightSuper = 347,
  Menu = 348,
  Last = 349,
};

enum class Action : std::uint8_t { Press, Release, Repeat };

enum class Mod : std::uint8_t {
  SHIFT = 0x1,
  CONTROL = 0x2,
  ALT = 0x4,
  SUPER = 0x8,
  CAPS_LOCK = 0x10,
  NUM_LOCK = 0x20,
};

// Enable bitwise operations for enum class Mod
inline Mod operator|(Mod lhs, Mod rhs) {
  return static_cast<Mod>(static_cast<std::uint8_t>(lhs) |
                          static_cast<std::uint8_t>(rhs));
}

inline Mod operator&(Mod lhs, Mod rhs) {
  return static_cast<Mod>(static_cast<std::uint8_t>(lhs) &
                          static_cast<std::uint8_t>(rhs));
}

inline Mod operator^(Mod lhs, Mod rhs) {
  return static_cast<Mod>(static_cast<std::uint8_t>(lhs) ^
                          static_cast<std::uint8_t>(rhs));
}

inline Mod operator~(Mod m) {
  return static_cast<Mod>(~static_cast<std::uint8_t>(m));
}

// Compound assignment operators (|=, &=, ^=)
inline Mod& operator|=(Mod& lhs, Mod rhs) {
  lhs = lhs | rhs;
  return lhs;
}

inline Mod& operator&=(Mod& lhs, Mod rhs) {
  lhs = lhs & rhs;
  return lhs;
}

inline Mod& operator^=(Mod& lhs, Mod rhs) {
  lhs = lhs ^ rhs;
  return lhs;
}

class MouseButtonEvent {
 public:
  struct Payload {
    MouseButton button;
    Action action;
    Mod mod;
  };

  explicit MouseButtonEvent(Payload&& v) : m_payload(std::move(v)) {}
  explicit MouseButtonEvent(const Payload& v) : m_payload(v) {}

  using payload_type = Payload;

  const Payload& payload() const { return m_payload; }

  constexpr bool canceled() const { return false; }

 private:
  [[no_unique_address]] Payload m_payload;
};

class MouseMoveEvent {
 public:
  using payload_type = vec2;

  explicit MouseMoveEvent(payload_type&& v) : m_payload(std::move(v)) {}
  explicit MouseMoveEvent(const payload_type& v) : m_payload(v) {}

  const payload_type& payload() const { return m_payload; }
  constexpr bool canceled() const { return false; }

 private:
  [[no_unique_address]] payload_type m_payload;
};

class MouseScrollEvent {
 public:
  using payload_type = dvec2;

  explicit MouseScrollEvent(payload_type&& v) : m_payload(std::move(v)) {}
  explicit MouseScrollEvent(const payload_type& v) : m_payload(v) {}

  const payload_type& payload() const { return m_payload; }
  constexpr bool canceled() const { return false; }

 private:
  [[no_unique_address]] payload_type m_payload;
};

class KeyboardEvent {
 public:
  struct Payload {
    Key key;
    Action action;
    Mod mod;
  };

  explicit KeyboardEvent(Payload&& v) : m_payload(std::move(v)) {}
  explicit KeyboardEvent(const Payload& v) : m_payload(v) {}

  using payload_type = Payload;

  const Payload& payload() const { return m_payload; }

  constexpr bool canceled() const { return false; }

 private:
  [[no_unique_address]] Payload m_payload;
};

class CharEvent {
 public:
  using payload_type = char32_t;

  explicit CharEvent(payload_type&& v) : m_payload(std::move(v)) {}
  explicit CharEvent(const payload_type& v) : m_payload(v) {}

  const payload_type& payload() const { return m_payload; }
  constexpr bool canceled() const { return false; }

 private:
  [[no_unique_address]] payload_type m_payload;
};

class ShutdownEvent {
 public:
  enum class State {
    CLOSED,
    EXITED,
  };
  using payload_type = State;
  explicit ShutdownEvent(payload_type&& v) : m_payload(std::move(v)) {}
  explicit ShutdownEvent(const payload_type& v) : m_payload(v) {}

  const payload_type& payload() const { return m_payload; }
  constexpr bool canceled() const { return false; }

 private:
  [[no_unique_address]] payload_type m_payload;
};

static_assert(events::Event<KeyboardEvent>);

class WindowImpl {
 public:
  WindowImpl(uvec2 size, std::string_view title,
             PolyAllocatorReference allocator);
  ~WindowImpl();

  WindowImpl(const WindowImpl&) = delete;
  WindowImpl& operator=(const WindowImpl&) = delete;
  WindowImpl(WindowImpl&&) = delete;
  WindowImpl& operator=(WindowImpl&&) = delete;

  void close();
  bool closed() const;

  void setSize(unsigned int widht, unsigned int height);
  uvec2 getSize() const;
  uvec2 getFramebufferSize() const;

  void setTitle(std::string_view title);
  std::string getTitle() const;

  void setResizable(bool resizable);
  bool isResizable() const;

  EventListenerHandle addKeyboardEventListener(
      EventListenerRef<KeyboardEvent> listener);

  EventListenerHandle addCharEventListener(
      EventListenerRef<CharEvent> listener);

  EventListenerHandle addResizeListener(EventListenerRef<ResizeEvent> listener);

  EventListenerHandle addFramebufferSizeListener(
      EventListenerRef<ResizeEvent> listener);

  EventListenerHandle addMouseButtonEventListener(
      EventListenerRef<MouseButtonEvent> listener);

  EventListenerHandle addMouseMoveEventListener(
      EventListenerRef<MouseMoveEvent> listener);

  EventListenerHandle addMouseScrollEventListener(
      EventListenerRef<MouseScrollEvent> listener);

  EventListenerHandle addShutdownEventListener(
      EventListenerRef<ShutdownEvent> listener);

 private:
  void* m_internals;
};

}  // namespace strobe::window
