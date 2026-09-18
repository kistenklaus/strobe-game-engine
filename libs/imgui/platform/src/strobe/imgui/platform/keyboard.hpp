#pragma once

#include "imgui.h"
#include "strobe/core/events/event_listener.hpp"
#include "strobe/core/events/event_listener_handle.hpp"
#include "strobe/platform/window.hpp"
#include "strobe/platform/window_events.hpp"

namespace strobe::imgui::platform {

class Keyboard {
private:
  static ImGuiKey to_imgui_key(strobe::platform::Key key) noexcept;

public:
  explicit Keyboard(strobe::platform::Window *window, ImGuiIO *io,
                    std::mutex *ioMutex) noexcept
      : m_io(io), m_ioMutex(ioMutex),
        m_keyListener(window->add_key_listener(
            EventListenerRef<strobe::platform::KeyEvent>::fromMemberFunction<
                Keyboard, &Keyboard::on_key>(this))),
        m_charListener(window->add_character_listener(
            EventListenerRef<strobe::platform::CharacterEvent>::
                fromMemberFunction<Keyboard, &Keyboard::on_char>(this))) {}
  ~Keyboard() noexcept {
    m_keyListener.release();
    m_charListener.release();
  }

  void on_key(const strobe::platform::KeyEvent &event) noexcept {
    if (event.action() == strobe::platform::Action::repeat) {
      return;
    }
    const bool down = event.action() != strobe::platform::Action::release;
    auto key = to_imgui_key(event.key());
    std::lock_guard lck{*m_ioMutex};
    m_io->AddKeyEvent(ImGuiMod_Ctrl,
                      bool(event.mod() & strobe::platform::Mod::control));

    m_io->AddKeyEvent(ImGuiMod_Shift,
                      bool(event.mod() & strobe::platform::Mod::shift));

    m_io->AddKeyEvent(ImGuiMod_Alt,
                      bool(event.mod() & strobe::platform::Mod::alt));

    m_io->AddKeyEvent(ImGuiMod_Super,
                      bool(event.mod() & strobe::platform::Mod::super));
    m_io->AddKeyEvent(key, down);
  }

  void on_char(const strobe::platform::CharacterEvent &event) noexcept {
    std::lock_guard lck{*m_ioMutex};
    m_io->AddInputCharacter(event.codepoint());
  }

private:
  ImGuiIO *m_io;
  std::mutex *m_ioMutex;
  EventListenerHandle m_keyListener;
  EventListenerHandle m_charListener;
};

} // namespace strobe::imgui::platform
