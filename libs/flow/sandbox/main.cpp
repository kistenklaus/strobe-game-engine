#include <strobe/core/sync/job_scheduler.hpp>
#include <strobe/flow/flow.hpp>

#include <cstdint>
#include <iostream>
#include <mutex>

namespace flow = strobe::flow;

namespace {

struct RenderSettings {
  int frame_step;

  bool operator==(const RenderSettings &) const = default;
};

void advance_frame(const flow::ExternalOpContext &context) noexcept {
  const auto previous = *static_cast<const int *>(context.arguments[0]);
  const auto &settings =
      *static_cast<const RenderSettings *>(context.arguments[1]);
  *static_cast<int *>(context.arguments[2]) = previous + settings.frame_step;
}

flow::ExternalOpFunction resolve_renderer(void *, std::string_view symbol,
                                          std::uint32_t abi) noexcept {
  return symbol == "advance-frame" && abi == 1 ? advance_frame : nullptr;
}

void submit_frames(flow::Program &program,
                   strobe::sync::JobScheduler &scheduler, std::uint64_t count) {
  program.release("frame", count);

  const auto completion = program.submit(&scheduler);
  scheduler.wait(completion);
  program.rethrow_execution_error();
}

void submit_once(flow::Program &program,
                 strobe::sync::JobScheduler &scheduler) {
  const auto completion = program.submit(&scheduler);
  scheduler.wait(completion);
  program.rethrow_execution_error();
}

} // namespace

int main() {
  RenderSettings settings{.frame_step = 1};
  std::mutex output_mutex;

  flow::begin(flow::CompileOptions{.max_slots = 3});
  flow::acquire("frame");

  auto initial_frame = flow::resource<int>();
  flow::construct(initial_frame, 0);
  auto external_settings = flow::external<RenderSettings>("render-settings");

  auto changed_settings = flow::cutoff(external_settings);
  flow::op(flow::read(changed_settings), [&](const RenderSettings &value) {
    std::lock_guard lock(output_mutex);
    std::cout << "rebuild settings-dependent resources: step="
              << value.frame_step << '\n';
  });

  auto previous_frame = flow::phi(initial_frame);
  auto current_frame = flow::resource<int>();
  flow::construct(current_frame, 0);
  flow::op(flow::external_op(
               "sandbox-renderer", "advance-frame", 1,
               flow::OpMetadata("Advance frame", "Computes the next frame")),
           flow::read(previous_frame), flow::read(changed_settings),
           flow::write(current_frame));
  flow::op(flow::read(current_frame), [&](const int &current) {
    std::lock_guard lock(output_mutex);
    std::cout << "render frame value: " << current << '\n';
  });
  flow::feedback(previous_frame, current_frame);

  auto frame_program = flow::end();
  frame_program.bind_external("render-settings", &settings);
  frame_program.bind_library(
      "sandbox-renderer",
      flow::Library{.state = nullptr, .resolve = resolve_renderer});

  bool use_then_branch = true;
  flow::begin();
  auto loop_counter = flow::resource<int>();
  flow::construct(loop_counter, 0);
  auto keep_looping = flow::resource<bool>();
  flow::construct(keep_looping, true);
  flow::while_(keep_looping, [&] {
    flow::op(flow::read_write(loop_counter), flow::write(keep_looping),
             [&](int &counter, bool &repeat) {
               ++counter;
               repeat = counter < 3;
               std::lock_guard lock(output_mutex);
               std::cout << "while iteration: " << counter << '\n';
             });
  });

  auto branch_condition = flow::external<bool>("use-then-branch");
  flow::if_(
      branch_condition,
      [&] {
        flow::op(flow::read(loop_counter), [&](const int &counter) {
          std::lock_guard lock(output_mutex);
          std::cout << "if selected the then branch after " << counter
                    << " iterations\n";
        });
      },
      [&] {
        flow::op(flow::read(loop_counter), [&](const int &counter) {
          std::lock_guard lock(output_mutex);
          std::cout << "if selected the else branch after " << counter
                    << " iterations\n";
        });
      });
  auto control_program = flow::end();
  control_program.bind_external("use-then-branch", &use_then_branch);

  strobe::sync::allocator allocator;
  strobe::sync::JobScheduler scheduler(&allocator, 3);

  std::cout << "operations: " << frame_program.operation_count() << '\n'
            << "resources: " << frame_program.resource_count() << '\n'
            << "activation capacity: "
            << frame_program.data().activation_layout.capacity << "\n\n";

  std::cout << "release three frame tokens\n";
  submit_frames(frame_program, scheduler, 3);

  {
    settings.frame_step = 2;
    std::cout << "\nchange settings and release two frame tokens\n";
  }
  submit_frames(frame_program, scheduler, 2);

  std::cout << "\nrun the structured control example\n";
  submit_once(control_program, scheduler);

  use_then_branch = false;
  std::cout << "\nselect the other branch and submit again\n";
  submit_once(control_program, scheduler);

  scheduler.stop();
}
