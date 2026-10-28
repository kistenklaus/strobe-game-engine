#pragma once

#include "strobe/flow/bool.hpp"
#include "strobe/flow/flow.hpp"
#include "strobe/flow/op_source.hpp"
#include "strobe/flow/resource.hpp"
#include "strobe/rhi/objects/timepoint.hpp"
#include <limits>
namespace strobe::rdg {

class Timepoint {
public:
  explicit Timepoint(flow::Resource<rhi::Timepoint> runtime) noexcept
      : m_runtime(runtime) {}

  void
  wait(uint64_t timeout = std::numeric_limits<uint64_t>::max()) const noexcept {
    flow::op(flow::intrinsic("strobe.rdg", "timepoint_wait", 1)
                 .immediate<uint64_t>(timeout),
             flow::read(m_runtime));
  }

  flow::boolean poll() const noexcept {
    auto sig = flow::resource<bool>();
    flow::construct(sig, flow::intrinsic("strobe.rdg", "timepoint_poll", 1),
                    flow::read(m_runtime));
    return sig;
  }

private:
  flow::Resource<rhi::Timepoint> m_runtime;
};

} // namespace strobe::rdg
