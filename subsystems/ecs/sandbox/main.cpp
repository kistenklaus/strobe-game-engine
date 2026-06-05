#include "strobe/core/type_traits/types.hpp"
#include "strobe/ecs/allocator.hpp"
#include "strobe/ecs/universe.hpp"
#include <chrono>
#include <fmt/printf.h>
#include <fmt/std.h>

using namespace strobe;

struct Foo {
  int x;
  explicit Foo(int x) noexcept : x(x) {
    // fmt::println("Consruct Foo with {}", x);
  }
  ~Foo() noexcept {
    // fmt::println("Destruct Foo with {}", x);
  }
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
  SystemB() : y(42) {
    // fmt::println("SystemB constructed");
  }
  ~SystemB() {
    // fmt::println("SystemB destructed");
  }

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
    rcmds.create<Bar>();
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
    rcmds.destroy<Bar>();
  }
};

int main() {
  // using ss = ecs::stateful_system<SystemB>;
  // using traits = ecs::system_traits<ss>;
  // static_assert(ecs::system_setup_exists_v<SystemA>);
  // static_assert(ecs::system_setup_exists_v<ecs::stateful_system<SystemB>>);

  for (uint32_t i = 0; i < 100000; ++i) {
    {
      using namespace strobe::ecs;
      allocator alloc{};
      Universe universe{&alloc, 8};

      fmt::println("initalized");

      for (uint32_t j = 0; j < 100; ++j) {
        universe.scheduler.submit(op_scope(acq_rel(universe.sr_location)),
                                  []() noexcept {});
      }
      universe.scheduler.fence(op_scope(acq_rel(universe.sr_location)));

      fmt::println("start");

      auto start = std::chrono::high_resolution_clock::now();

      uint32_t IT = 2;
      for (uint32_t i = 0; i < IT; ++i) {
        fmt::println("\niteration");

        universe.scheduler.submit(
            op_scope(acq_rel(universe.sr_location)),
            [&universe]() noexcept { universe.rreg.cmd_emplace<Foo>(1); });

        universe.scheduler.submit(
            op_scope(acq_rel(universe.sr_location)),
            [&universe]() noexcept { universe.sreg.cmd_create<SystemA>(); });

        drain_cmds(&universe.scheduler, &universe.sr_domain,
                   op_scope(acq_rel(universe.sr_location)),
                   universe.sreg.cmds(), universe.rreg.cmds());

        universe.schedule.submit_all(&universe.scheduler);

        universe.scheduler.submit(op_scope(acq_rel(universe.sr_location)),
                                  [&universe]() noexcept {
                                    universe.sreg.cmd_destroy<SystemA>();
                                    universe.rreg.cmd_destroy<Foo>();
                                  });

        drain_cmds(&universe.scheduler, &universe.sr_domain,
                   op_scope(acq_rel(universe.sr_location)),
                   universe.sreg.cmds(), universe.rreg.cmds());
      }

      auto end = std::chrono::high_resolution_clock::now();
      fmt::println(
          "average iteration took {}",
          std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(
              end - start) /
              IT);
    }
  }
  fmt::println("exit");
}
