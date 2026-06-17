#include "strobe/core/memory/named_allocator.hpp"
#include "strobe/core/type_traits/fixed_string.hpp"
#include "strobe/core/type_traits/types.hpp"
#include "strobe/ecs/allocator.hpp"
#include "strobe/ecs/ecs.hpp"
#include "strobe/ecs/resource_commands.hpp"
#include "strobe/ecs/system/system_traits.hpp"
#include "strobe/ecs/universe.hpp"
#include "tracy/Tracy.hpp"
#include <chrono>
#include <common/TracySystem.hpp>
#include <fmt/printf.h>
#include <fmt/std.h>
#include <thread>

using namespace strobe;

struct Foo {
  int x;
  explicit Foo(int x) noexcept : x(x) {
    fmt::println("Consruct Foo with {}", x);
  }
  ~Foo() noexcept { fmt::println("Destruct Foo with {}", x); }
};

struct Bar {
  int y = 0;
  explicit Bar() noexcept {
    // fmt::println("Construct Bar");
  }
  ~Bar() noexcept {
    // fmt::println("Destruct Bar");
  }
};

// struct SystemB {
//
//   int y;
//   SystemB() : y(42) { fmt::println("SystemB constructed"); }
//   ~SystemB() { fmt::println("SystemB destructed"); }
//
//   void setup(ResourceCommands rcmds, Resource<const Foo>) noexcept {
//     fmt::println("B setup");
//     rcmds.enable<SystemB>();
//     rcmds.create<Bar>();
//   }
//
//   void start(Resource<const Bar>, Resource<const Foo>) noexcept {
//     fmt::println("B start");
//   }
//
//   void update(Resource<const Foo>) noexcept { fmt::println("B update"); }
//
//   void stop(Resource<const Bar>, Resource<const Foo>) noexcept {
//     fmt::println("B stop");
//   }
//
//   void teardown([[maybe_unused]] ResourceCommands rcmds) noexcept {
//     fmt::println("B teardown");
//     rcmds.destroy<Bar>();
//   }
// };

struct MiniSystem {
  void setup() noexcept {}
  void teardown() noexcept {}
};

struct SystemA {

  // using sequenced_after = Types<SystemB>;

  void setup(ResourceCommands rcmds,
             [[maybe_unused]] Resource<const Foo> foo) const noexcept {
    fmt::println("A setup");
    rcmds.enable<SystemA>();
    // rcmds.create<SystemB>();
    // rcmds.create<Bar>();
  }

  void start(Resource<const Foo>) noexcept { fmt::println("A start"); }

  void update([[maybe_unused]] Resource<const Foo> foo, ResourceCommands rcmds,
              TaskCommands tcmds) noexcept {
    rcmds.create<MiniSystem>();
    rcmds.destroy<MiniSystem>();
    rcmds.create<Bar>();
    rcmds.destroy<Bar>();

    tcmds.submit([]() noexcept {});
    // bar->y += 1;
  }

  void stop(Resource<const Foo>) noexcept { fmt::println("A stop"); }

  void teardown([[maybe_unused]] ResourceCommands rcmds,
                [[maybe_unused]] Resource<const Foo> foo) noexcept {
    fmt::println("A teardown");
    // rcmds.destroy<SystemB>();
    // rcmds.destroy<Bar>();
  }
};

// tracy playground

int main() {
  while (!TracyIsConnected) {
    std::this_thread::yield();
  }
  fmt::println("connected");

  ECS ecs{[](ResourceCommands rcmds) noexcept {
            fmt::println("Hello");
            rcmds.create<Foo>(1);
            rcmds.create<SystemA>();
          },
          2};

  // std::this_thread::sleep_for(std::chrono::duration<float>(1));
  // ecs.stop();
  ecs.join();
}
