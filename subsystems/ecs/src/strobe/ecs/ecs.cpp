#include "strobe/ecs/ecs.hpp"
#include "strobe/core/memory/Mallocator.hpp"
#include "strobe/ecs/barrier.hpp"
#include "strobe/ecs/commands.hpp"
#include "strobe/ecs/query.hpp"
#include "strobe/ecs/resource.hpp"
#include "strobe/ecs/schedule/job_scheduler.hpp"
#include "strobe/ecs/schedule/sequencer.hpp"
#include "strobe/ecs/system_traits.hpp"
#include "strobe/ecs/task_traits.hpp"
#include <fmt/printf.h>
#include <stdatomic.h>
#include <strobe/core/containers/vector_storage.hpp>

using namespace strobe;

struct OtherSystem {};
struct AnotherSystem {};
struct PostEventBarrier;

struct UserController {};
struct Time {};

struct ExampleSystem {
  using run_after = Types<AnotherSystem, strobe::Barrier<PostEventBarrier>>;
  using run_before = Types<OtherSystem>;

  void start(ResourceCommands resources) const {
    resources.createResource<Time>({});
  }

  void update(Query<const UserController &> userController,
              Resource<const Time> time) const {}

  void stop(Resource<const Time> time) const {}
};

struct Renderer {
  Renderer() {
    // initialize renderer backend
  }
  ~Renderer() {
    // teardown renderer backend
  }
};
struct Window {
  Window() {
    // initialize window backend
  }
  ~Window() {
    // teardown window backend
  }
};
struct RendererSystem {

  void setup(ResourceCommands resources, Resource<Window>) {
    resources.createResource(Renderer{});
  }

  void update(Resource<Renderer>, ...);

  void teardown(ResourceCommands resources, Resource<Window>) {
    resources.destroyResource<Renderer>();
  }
};

struct WindowSystem {
  void setup(ResourceCommands resources) { resources.createResource(Window{}); }

  void update(Resource<Window> window);

  void teardown(ResourceCommands resources) {
    resources.destroyResource<Window>();
  }
};

struct ExampleTask {

  void operator()(ResourceCommands resources /* ... */) {}
};

struct AsyncTaskExample {

  auto operator()() {
    return [](AsyncTaskExample &&self, ResourceCommands &commands) {
      // executed when the ecs completes this tasks, can be used to apply
      // changes to the ECS.
    };
  }
};

int foo() {

  strobe::Mallocator malloc;
  ecs::JobScheduler scheduler(&malloc, 4, 8);
  return 0;
}
