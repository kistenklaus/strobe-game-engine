

#pragma once

#include "strobe/flow/access.hpp"
#include "strobe/flow/flow.hpp"
#include "strobe/flow/op_source.hpp"
#include "strobe/rhi/objects/device.hpp"
#include "strobe/rhi/objects/queue.hpp"
#include "strobe/rhi/types/queue_flags.hpp"
#include <memory>
namespace strobe::rdg {

inline void begin() {
  auto device = flow::external<rhi::Device>("device");
  auto queue = flow::resource<rhi::Queue>();
  flow::construct(
      queue, flow::OpMetadata{"get_queue"},
      flow::read(device),
      [](rhi::Queue *queue, const rhi::Device &device) {
        std::construct_at(queue, device.get_queue(rhi::QueueFlags::compute |
                                                  rhi::QueueFlags::transfer |
                                                  rhi::QueueFlags::graphics |
                                                  rhi::QueueFlags::present));
      });
  flow::op(flow::intrinsic("strobe.rdg", "def_device", 1),
           flow::read_write(device));
  flow::op(flow::intrinsic("strobe.rdg", "def_queue", 1),
           flow::read_write(queue));
}

inline void end() {}

} // namespace strobe::rdg
