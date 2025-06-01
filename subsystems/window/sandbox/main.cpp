#include <GLFW/glfw3.h>

#include <print>
#include <strobe/memory.hpp>
#include <strobe/window/WindowManager.hpp>
#include <thread>

#include "strobe/core/memory/Mallocator.hpp"
#include "strobe/window/KeyboardEvent.hpp"

using namespace strobe;
using namespace std::chrono_literals;

int main() {
  WindowManager subsys{};

  WindowHandle win = subsys.createWindow(uvec2(800, 600), "idk", false);
  auto handle = win->addKeyboardEventListener(
      EventListenerRef<window::KeyboardEvent>::fromNative(
          nullptr, [](void*, const window::KeyboardEvent& event) {
            std::println("Keyboard event");
          }));

  while (!win->closed()) {
    std::this_thread::yield();
  }
}
 
// no inline, required by [replacement.functions]/3
void* operator new(std::size_t sz)
{
    std::printf("1) new(size_t), size = %zu\n", sz);
    if (sz == 0)
        ++sz; // avoid std::malloc(0) which may return nullptr on success
 
    if (void *ptr = std::malloc(sz))
        return ptr;
 
    throw std::bad_alloc{}; // required by [new.delete.single]/3
}
 
// no inline, required by [replacement.functions]/3
void* operator new[](std::size_t sz)
{
    std::printf("2) new[](size_t), size = %zu\n", sz);
    if (sz == 0)
        ++sz; // avoid std::malloc(0) which may return nullptr on success
 
    if (void *ptr = std::malloc(sz))
        return ptr;
 
    throw std::bad_alloc{}; // required by [new.delete.single]/3
}
 
