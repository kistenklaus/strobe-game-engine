#include "strobe/flow/channel.hpp"
#include "strobe/flow/loop.hpp"
#include <strobe/flow/flow.hpp>

struct Frame {
  int id;
};

int main() {
  using namespace strobe;

  flow::begin();

  using namespace strobe;

  flow::loop(
      [&]() noexcept {
        auto frame = flow::recv<Frame>("fchan");
        flow::op(flow::read(frame), [](const Frame &frame) noexcept {
          fmt::println("frame: {}", frame.id);
        });
      },
      2);

  auto prog = flow::end();

  flow::Channel<Frame> fchan{&prog, "fchan"};

  sync::allocator alloc;
  sync::JobScheduler scheduler(&alloc, 4);

  int i = 0;
  while (true) {
    fchan.send(Frame{.id = i++});
    prog.submit(&scheduler);
  }

  return 0;
}
