
#include "strobe/core/events/event_listener.hpp"
#include "strobe/core/lina/vec.hpp"
#include "strobe/platform/window.hpp"
#include "strobe/platform/window_events.hpp"
#include <fmt/printf.h>

int main() {
  strobe::platform::Window window{
      strobe::uvec2{800, 600},
      "Hello World!",
  };
  window.visible(true);

  auto callback = [&](strobe::platform::KeyEvent event) {
    fmt::println("key = {}", static_cast<uint32_t>(event.key()));
    if (event.key() == strobe::platform::Key::escape) {
      window.should_close(true);
    }
  };

  auto listener = window.add_key_listener(
      strobe::EventListenerRef<strobe::platform::KeyEvent>::fromCallable(
          &callback));
  while (!window.should_close()) {
  }
}
