#pragma once

#include "strobe/rhi/objects/device.hpp"
#include "strobe/rhi/objects/sampler_descriptor.hpp"
namespace strobe::imgui {

struct Samplers {
  explicit Samplers(rhi::Device &device) noexcept;

  const rhi::SamplerDescriptor linearSampler() const noexcept {
    return m_linearSampler;
  }
  const rhi::SamplerDescriptor nearestSampler() const noexcept {
    return m_linearSampler;
  }
private:
  rhi::SamplerDescriptor m_linearSampler;
  rhi::SamplerDescriptor m_nearestSampler;
};

} // namespace strobe::imgui
