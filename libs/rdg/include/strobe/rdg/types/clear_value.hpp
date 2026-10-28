#pragma once

#include "strobe/flow/flow.hpp"
#include "strobe/flow/vec.hpp"
#include "strobe/rhi/types/clear_value.hpp"
#include <memory>

namespace strobe::rdg {

class ClearValue {
public:
  ClearValue(rhi::ClearValue value) noexcept
      : m_runtime(flow::resource<rhi::ClearValue>()) {
    flow::construct(m_runtime, value);
  }

  static ClearValue float4(flow::vec4 float4) {
    auto runtime = flow::resource<rhi::ClearValue>();
    flow::construct(runtime, flow::read(float4.runtime()),
                    [](rhi::ClearValue *ptr, strobe::vec4 float4) {
                    ZoneScopedN("ClearValue::float4");
                      std::construct_at(ptr, rhi::ClearValue{
                                                 .float4 = float4,
                                             });
                    });
    return ClearValue{runtime};
  }

  ClearValue(flow::Resource<rhi::ClearValue> runtime) noexcept
      : m_runtime(runtime) {}

  flow::Resource<rhi::ClearValue> runtime() const noexcept { return m_runtime; }

private:
  flow::Resource<rhi::ClearValue> m_runtime;
};

} // namespace strobe::rdg
