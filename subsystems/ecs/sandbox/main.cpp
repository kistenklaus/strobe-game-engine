#include "strobe/core/type_traits/types.hpp"
#include "strobe/ecs/allocator.hpp"
#include "strobe/ecs/ecs.hpp"
#include "strobe/ecs/resource_commands.hpp"
#include "strobe/ecs/universe.hpp"
#include <chrono>
#include <fmt/printf.h>
#include <fmt/std.h>

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

struct SystemB {

  int y;
  SystemB() : y(42) { fmt::println("SystemB constructed"); }
  ~SystemB() { fmt::println("SystemB destructed"); }

  void setup(ResourceCommands rcmds, Resource<const Foo>) noexcept {
    fmt::println("B setup");
    rcmds.enable<SystemB>();
  }

  void start(Resource<const Bar>, Resource<const Foo>) noexcept {
    fmt::println("B start");
  }

  void update(Resource<const Foo>) noexcept { fmt::println("B update"); }

  void stop(Resource<const Bar>, Resource<const Foo>) noexcept {
    fmt::println("B stop");
  }

  void teardown([[maybe_unused]] ResourceCommands rcmds) noexcept {
    fmt::println("B teardown");
    // rcmds.destroy<Bar>();
  }
};

struct SystemA {

  using sequenced_after = Types<SystemB>;

  void setup(ResourceCommands rcmds,
             [[maybe_unused]] Resource<const Foo> foo) const noexcept {
    fmt::println("A setup");
    rcmds.enable<SystemA>();
    rcmds.create<SystemB>();
    // rcmds.create<Bar>();
  }

  void start(Resource<const Foo>) noexcept { fmt::println("A start"); }

  void update([[maybe_unused]] Resource<const Foo> foo) noexcept {
    // bar->y += 1;
    fmt::println("A update");
  }

  void stop(Resource<const Foo>) noexcept { fmt::println("A stop"); }

  void teardown([[maybe_unused]] ResourceCommands rcmds,
                [[maybe_unused]] Resource<const Foo> foo) noexcept {
    fmt::println("A teardown");
    rcmds.destroy<SystemB>();
    // rcmds.destroy<Bar>();
  }
};

int main() {

  strobe::ECS ecs{[](ResourceCommands rcmds, TaskCommands tcmds) noexcept {
    fmt::println("HI");
    tcmds.submit(
        [](TaskCommands tcmds, Resource<Foo> foo) noexcept { 
        fmt::println("Second HI"); 
        });
    rcmds.create<SystemA>();
    rcmds.enable<SystemA>();
  }};
}
