#pragma once

#include <cstdint>
#include <span>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

namespace strobe::platform {

// -----------------------------------------------------------------------------
// Common event state
// -----------------------------------------------------------------------------

class EventState {
public:
  [[nodiscard]] bool canceled() const noexcept { return m_canceled; }

  void consume() const noexcept { m_canceled = true; }

private:
  mutable bool m_canceled = false;
};

// -----------------------------------------------------------------------------
// Enumerations
// -----------------------------------------------------------------------------

enum class Key : std::int16_t {
  unknown = -1,

  space,
  apostrophe,
  comma,
  minus,
  period,
  slash,

  num_0,
  num_1,
  num_2,
  num_3,
  num_4,
  num_5,
  num_6,
  num_7,
  num_8,
  num_9,

  semicolon,
  equal,

  a,
  b,
  c,
  d,
  e,
  f,
  g,
  h,
  i,
  j,
  k,
  l,
  m,
  n,
  o,
  p,
  q,
  r,
  s,
  t,
  u,
  v,
  w,
  x,
  y,
  z,

  left_bracket,
  backslash,
  right_bracket,
  grave_accent,

  world_1,
  world_2,

  escape,
  enter,
  tab,
  backspace,
  insert,
  del,

  right,
  left,
  down,
  up,

  page_up,
  page_down,
  home,
  end,

  caps_lock,
  scroll_lock,
  num_lock,
  print_screen,
  pause,

  f1,
  f2,
  f3,
  f4,
  f5,
  f6,
  f7,
  f8,
  f9,
  f10,
  f11,
  f12,
  f13,
  f14,
  f15,
  f16,
  f17,
  f18,
  f19,
  f20,
  f21,
  f22,
  f23,
  f24,
  f25,

  keypad_0,
  keypad_1,
  keypad_2,
  keypad_3,
  keypad_4,
  keypad_5,
  keypad_6,
  keypad_7,
  keypad_8,
  keypad_9,

  keypad_decimal,
  keypad_divide,
  keypad_multiply,
  keypad_subtract,
  keypad_add,
  keypad_enter,
  keypad_equal,

  left_shift,
  left_control,
  left_alt,
  left_super,

  right_shift,
  right_control,
  right_alt,
  right_super,

  menu,
};

enum class Mod : std::uint8_t {
  none = 0,
  shift = 1u << 0,
  control = 1u << 1,
  alt = 1u << 2,
  super = 1u << 3,
  caps_lock = 1u << 4,
  num_lock = 1u << 5,
};

[[nodiscard]] constexpr Mod operator|(Mod lhs, Mod rhs) noexcept {
  return static_cast<Mod>(static_cast<std::underlying_type_t<Mod>>(lhs) |
                          static_cast<std::underlying_type_t<Mod>>(rhs));
}

[[nodiscard]] constexpr Mod operator&(Mod lhs, Mod rhs) noexcept {
  return static_cast<Mod>(static_cast<std::underlying_type_t<Mod>>(lhs) &
                          static_cast<std::underlying_type_t<Mod>>(rhs));
}

[[nodiscard]] constexpr Mod operator^(Mod lhs, Mod rhs) noexcept {
  return static_cast<Mod>(static_cast<std::underlying_type_t<Mod>>(lhs) ^
                          static_cast<std::underlying_type_t<Mod>>(rhs));
}

[[nodiscard]] constexpr Mod operator~(Mod value) noexcept {
  return static_cast<Mod>(~static_cast<std::underlying_type_t<Mod>>(value));
}

constexpr Mod &operator|=(Mod &lhs, Mod rhs) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

constexpr Mod &operator&=(Mod &lhs, Mod rhs) noexcept {
  lhs = lhs & rhs;
  return lhs;
}

constexpr Mod &operator^=(Mod &lhs, Mod rhs) noexcept {
  lhs = lhs ^ rhs;
  return lhs;
}

[[nodiscard]] constexpr bool
operator==(Mod lhs, std::underlying_type_t<Mod> rhs) noexcept {
  return static_cast<std::underlying_type_t<Mod>>(lhs) == rhs;
}

[[nodiscard]] constexpr bool
operator!=(Mod lhs, std::underlying_type_t<Mod> rhs) noexcept {
  return static_cast<std::underlying_type_t<Mod>>(lhs) != rhs;
}

[[nodiscard]] constexpr bool operator==(std::underlying_type_t<Mod> lhs,
                                        Mod rhs) noexcept {
  return lhs == static_cast<std::underlying_type_t<Mod>>(rhs);
}

[[nodiscard]] constexpr bool operator!=(std::underlying_type_t<Mod> lhs,
                                        Mod rhs) noexcept {
  return lhs != static_cast<std::underlying_type_t<Mod>>(rhs);
}

enum class Action : std::uint8_t {
  release,
  press,
  repeat,
};

// -----------------------------------------------------------------------------
// Window events
// -----------------------------------------------------------------------------

struct WindowPositionEvent : EventState {
  using payload_type = std::tuple<int, int>;

  explicit WindowPositionEvent(int x, int y) noexcept : m_payload(x, y) {}

  explicit WindowPositionEvent(payload_type payload) noexcept
      : m_payload(std::move(payload)) {}

  const payload_type &payload() const noexcept { return m_payload; }

  int x() const noexcept { return std::get<0>(m_payload); }

  int y() const noexcept { return std::get<1>(m_payload); }

private:
  payload_type m_payload;
};

struct WindowSizeEvent : EventState {
  using payload_type = std::tuple<int, int>;

  explicit WindowSizeEvent(int width, int height) noexcept
      : m_payload(width, height) {}

  explicit WindowSizeEvent(payload_type payload) noexcept
      : m_payload(std::move(payload)) {}

  const payload_type &payload() const noexcept { return m_payload; }

  int width() const noexcept { return std::get<0>(m_payload); }

  int height() const noexcept { return std::get<1>(m_payload); }

private:
  payload_type m_payload;
};

struct WindowCloseEvent : EventState {
  using payload_type = std::monostate;

  explicit WindowCloseEvent(payload_type = {}) noexcept {}

  const payload_type &payload() const noexcept { return m_payload; }

private:
  payload_type m_payload{};
};

struct WindowRefreshEvent : EventState {
  using payload_type = std::monostate;

  explicit WindowRefreshEvent(payload_type = {}) noexcept {}

  const payload_type &payload() const noexcept { return m_payload; }

private:
  payload_type m_payload{};
};

struct WindowFocusEvent : EventState {
  using payload_type = bool;

  explicit WindowFocusEvent(payload_type payload) noexcept
      : m_payload(payload) {}

  const payload_type &payload() const noexcept { return m_payload; }

  bool focused() const noexcept { return m_payload; }

private:
  payload_type m_payload;
};

struct WindowIconifyEvent : EventState {
  using payload_type = bool;

  explicit WindowIconifyEvent(payload_type payload) noexcept
      : m_payload(payload) {}

  const payload_type &payload() const noexcept { return m_payload; }

  bool iconified() const noexcept { return m_payload; }

private:
  payload_type m_payload;
};

struct WindowMaximizeEvent : EventState {
  using payload_type = bool;

  explicit WindowMaximizeEvent(payload_type payload) noexcept
      : m_payload(payload) {}

  const payload_type &payload() const noexcept { return m_payload; }

  bool maximized() const noexcept { return m_payload; }

private:
  payload_type m_payload;
};

struct FramebufferSizeEvent : EventState {
  using payload_type = std::tuple<int, int>;

  explicit FramebufferSizeEvent(int width, int height) noexcept
      : m_payload(width, height) {}

  explicit FramebufferSizeEvent(payload_type payload) noexcept
      : m_payload(std::move(payload)) {}

  const payload_type &payload() const noexcept { return m_payload; }

  int width() const noexcept { return std::get<0>(m_payload); }

  int height() const noexcept { return std::get<1>(m_payload); }

private:
  payload_type m_payload;
};

struct WindowContentScaleEvent : EventState {
  using payload_type = std::tuple<float, float>;

  explicit WindowContentScaleEvent(float xscale, float yscale) noexcept
      : m_payload(xscale, yscale) {}

  explicit WindowContentScaleEvent(payload_type payload) noexcept
      : m_payload(std::move(payload)) {}

  const payload_type &payload() const noexcept { return m_payload; }

  float xscale() const noexcept { return std::get<0>(m_payload); }

  float yscale() const noexcept { return std::get<1>(m_payload); }

private:
  payload_type m_payload;
};

// -----------------------------------------------------------------------------
// Keyboard events
// -----------------------------------------------------------------------------

struct KeyEvent : EventState {
  using payload_type = std::tuple<Key, int, Mod, Action>;

  explicit KeyEvent(Key key, int scancode, Mod mod, Action action) noexcept
      : m_payload(key, scancode, mod, action) {}

  explicit KeyEvent(payload_type payload) noexcept
      : m_payload(std::move(payload)) {}

  const payload_type &payload() const noexcept { return m_payload; }

  Key key() const noexcept { return std::get<0>(m_payload); }

  int scancode() const noexcept { return std::get<1>(m_payload); }

  Mod mod() const noexcept { return std::get<2>(m_payload); }

  Action action() const noexcept { return std::get<3>(m_payload); }

private:
  payload_type m_payload;
};

struct CharacterEvent : EventState {
  using payload_type = char32_t;

  explicit CharacterEvent(payload_type payload) noexcept : m_payload(payload) {}

  const payload_type &payload() const noexcept { return m_payload; }

  char32_t codepoint() const noexcept { return m_payload; }

private:
  payload_type m_payload;
};

struct CharacterModsEvent : EventState {
  using payload_type = std::tuple<char32_t, Mod>;

  explicit CharacterModsEvent(char32_t codepoint, Mod mod) noexcept
      : m_payload(codepoint, mod) {}

  explicit CharacterModsEvent(payload_type payload) noexcept
      : m_payload(std::move(payload)) {}

  const payload_type &payload() const noexcept { return m_payload; }

  char32_t codepoint() const noexcept { return std::get<0>(m_payload); }

  Mod mod() const noexcept { return std::get<1>(m_payload); }

private:
  payload_type m_payload;
};

// -----------------------------------------------------------------------------
// Mouse events
// -----------------------------------------------------------------------------

struct CursorPositionEvent : EventState {
  using payload_type = std::tuple<double, double>;

  explicit CursorPositionEvent(double xpos, double ypos) noexcept
      : m_payload(xpos, ypos) {}

  explicit CursorPositionEvent(payload_type payload) noexcept
      : m_payload(std::move(payload)) {}

  const payload_type &payload() const noexcept { return m_payload; }

  double x() const noexcept { return std::get<0>(m_payload); }

  double y() const noexcept { return std::get<1>(m_payload); }

private:
  payload_type m_payload;
};

struct CursorEnterEvent : EventState {
  using payload_type = bool;

  explicit CursorEnterEvent(payload_type payload) noexcept
      : m_payload(payload) {}

  const payload_type &payload() const noexcept { return m_payload; }

  bool entered() const noexcept { return m_payload; }

private:
  payload_type m_payload;
};

struct MouseButtonEvent : EventState {
  using payload_type = std::tuple<int, Action, Mod>;

  explicit MouseButtonEvent(int button, Action action, Mod mod) noexcept
      : m_payload(button, action, mod) {}

  explicit MouseButtonEvent(payload_type payload) noexcept
      : m_payload(std::move(payload)) {}

  const payload_type &payload() const noexcept { return m_payload; }

  int button() const noexcept { return std::get<0>(m_payload); }

  Action action() const noexcept { return std::get<1>(m_payload); }

  Mod mod() const noexcept { return std::get<2>(m_payload); }

private:
  payload_type m_payload;
};

struct ScrollEvent : EventState {
  using payload_type = std::tuple<double, double>;

  explicit ScrollEvent(double xoffset, double yoffset) noexcept
      : m_payload(xoffset, yoffset) {}

  explicit ScrollEvent(payload_type payload) noexcept
      : m_payload(std::move(payload)) {}

  const payload_type &payload() const noexcept { return m_payload; }

  double xoffset() const noexcept { return std::get<0>(m_payload); }

  double yoffset() const noexcept { return std::get<1>(m_payload); }

private:
  payload_type m_payload;
};

// -----------------------------------------------------------------------------
// File-drop events
// -----------------------------------------------------------------------------

struct DropEvent : EventState {
  // GLFW owns these strings. They are only valid during the callback.
  using payload_type = std::span<const char *const>;

  explicit DropEvent(int count, const char **paths) noexcept
      : m_payload(paths, static_cast<std::size_t>(count)) {}

  explicit DropEvent(payload_type payload) noexcept : m_payload(payload) {}

  const payload_type &payload() const noexcept { return m_payload; }

  std::size_t count() const noexcept { return m_payload.size(); }

  const char *path(std::size_t index) const noexcept {
    return m_payload[index];
  }

  payload_type paths() const noexcept { return m_payload; }

private:
  payload_type m_payload;
};

} // namespace strobe::platform
