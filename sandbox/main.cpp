#include "strobe/ecs/ecs.hpp"
#include "strobe/platform/platform.hpp"
#include <strobe/core/fs/Path.hpp>
#include <strobe/core/fs/mkdir.hpp>
#include <strobe/core/memory/Mallocator.hpp>
#include <strobe/window/window.hpp>

int main() {
  using MainWindow = strobe::Window<{"main"}>;
  // using SecondWindow = strobe::Window<{"second"}>;

  strobe::ECS ecs{[](strobe::ResourceCommands rcmds) noexcept {
    rcmds.create<MainWindow>();
    rcmds.enable<MainWindow>();

    // rcmds.create<SecondWindow>();
    // rcmds.enable<SecondWindow>();
  }};

  strobe::Platform::native_main();

  ecs.join();
}
