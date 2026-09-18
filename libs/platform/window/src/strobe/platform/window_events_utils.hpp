#pragma once

#include "strobe/platform/window_events.hpp"

#include <GLFW/glfw3.h>

#include <cassert>
#include <type_traits>

namespace strobe::platform::detail {

inline Key from_glfw_key(int key) noexcept {
  using Underlying = std::underlying_type_t<Key>;

  if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9) {
    return static_cast<Key>(static_cast<Underlying>(Key::num_0) +
                            (key - GLFW_KEY_0));
  }

  if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) {
    return static_cast<Key>(static_cast<Underlying>(Key::a) +
                            (key - GLFW_KEY_A));
  }

  if (key >= GLFW_KEY_F1 && key <= GLFW_KEY_F25) {
    return static_cast<Key>(static_cast<Underlying>(Key::f1) +
                            (key - GLFW_KEY_F1));
  }

  if (key >= GLFW_KEY_KP_0 && key <= GLFW_KEY_KP_9) {
    return static_cast<Key>(static_cast<Underlying>(Key::keypad_0) +
                            (key - GLFW_KEY_KP_0));
  }

  switch (key) {
  case GLFW_KEY_UNKNOWN:
    return Key::unknown;

  case GLFW_KEY_SPACE:
    return Key::space;
  case GLFW_KEY_APOSTROPHE:
    return Key::apostrophe;
  case GLFW_KEY_COMMA:
    return Key::comma;
  case GLFW_KEY_MINUS:
    return Key::minus;
  case GLFW_KEY_PERIOD:
    return Key::period;
  case GLFW_KEY_SLASH:
    return Key::slash;
  case GLFW_KEY_SEMICOLON:
    return Key::semicolon;
  case GLFW_KEY_EQUAL:
    return Key::equal;

  case GLFW_KEY_LEFT_BRACKET:
    return Key::left_bracket;
  case GLFW_KEY_BACKSLASH:
    return Key::backslash;
  case GLFW_KEY_RIGHT_BRACKET:
    return Key::right_bracket;
  case GLFW_KEY_GRAVE_ACCENT:
    return Key::grave_accent;

  case GLFW_KEY_WORLD_1:
    return Key::world_1;
  case GLFW_KEY_WORLD_2:
    return Key::world_2;

  case GLFW_KEY_ESCAPE:
    return Key::escape;
  case GLFW_KEY_ENTER:
    return Key::enter;
  case GLFW_KEY_TAB:
    return Key::tab;
  case GLFW_KEY_BACKSPACE:
    return Key::backspace;
  case GLFW_KEY_INSERT:
    return Key::insert;
  case GLFW_KEY_DELETE:
    return Key::del;

  case GLFW_KEY_RIGHT:
    return Key::right;
  case GLFW_KEY_LEFT:
    return Key::left;
  case GLFW_KEY_DOWN:
    return Key::down;
  case GLFW_KEY_UP:
    return Key::up;

  case GLFW_KEY_PAGE_UP:
    return Key::page_up;
  case GLFW_KEY_PAGE_DOWN:
    return Key::page_down;
  case GLFW_KEY_HOME:
    return Key::home;
  case GLFW_KEY_END:
    return Key::end;

  case GLFW_KEY_CAPS_LOCK:
    return Key::caps_lock;
  case GLFW_KEY_SCROLL_LOCK:
    return Key::scroll_lock;
  case GLFW_KEY_NUM_LOCK:
    return Key::num_lock;
  case GLFW_KEY_PRINT_SCREEN:
    return Key::print_screen;
  case GLFW_KEY_PAUSE:
    return Key::pause;

  case GLFW_KEY_KP_DECIMAL:
    return Key::keypad_decimal;
  case GLFW_KEY_KP_DIVIDE:
    return Key::keypad_divide;
  case GLFW_KEY_KP_MULTIPLY:
    return Key::keypad_multiply;
  case GLFW_KEY_KP_SUBTRACT:
    return Key::keypad_subtract;
  case GLFW_KEY_KP_ADD:
    return Key::keypad_add;
  case GLFW_KEY_KP_ENTER:
    return Key::keypad_enter;
  case GLFW_KEY_KP_EQUAL:
    return Key::keypad_equal;

  case GLFW_KEY_LEFT_SHIFT:
    return Key::left_shift;
  case GLFW_KEY_LEFT_CONTROL:
    return Key::left_control;
  case GLFW_KEY_LEFT_ALT:
    return Key::left_alt;
  case GLFW_KEY_LEFT_SUPER:
    return Key::left_super;

  case GLFW_KEY_RIGHT_SHIFT:
    return Key::right_shift;
  case GLFW_KEY_RIGHT_CONTROL:
    return Key::right_control;
  case GLFW_KEY_RIGHT_ALT:
    return Key::right_alt;
  case GLFW_KEY_RIGHT_SUPER:
    return Key::right_super;

  case GLFW_KEY_MENU:
    return Key::menu;

  default:
    assert(false && "Unknown GLFW key code.");
    return Key::unknown;
  }
}

inline Mod from_glfw_mods(int mods) noexcept {
  Mod result = Mod::none;

  if ((mods & GLFW_MOD_SHIFT) != 0) {
    result |= Mod::shift;
  }

  if ((mods & GLFW_MOD_CONTROL) != 0) {
    result |= Mod::control;
  }

  if ((mods & GLFW_MOD_ALT) != 0) {
    result |= Mod::alt;
  }

  if ((mods & GLFW_MOD_SUPER) != 0) {
    result |= Mod::super;
  }

  if ((mods & GLFW_MOD_CAPS_LOCK) != 0) {
    result |= Mod::caps_lock;
  }

  if ((mods & GLFW_MOD_NUM_LOCK) != 0) {
    result |= Mod::num_lock;
  }

  return result;
}

inline Action from_glfw_action(int action) noexcept {
  switch (action) {
  case GLFW_RELEASE:
    return Action::release;

  case GLFW_PRESS:
    return Action::press;

  case GLFW_REPEAT:
    return Action::repeat;

  default:
    assert(false && "Unknown GLFW action.");
    return Action::release;
  }
}

} // namespace strobe::platform::detail
