#include "strobe/imgui/renderer/samplers.hpp"

namespace strobe::imgui::renderer {

Samplers::Samplers(rhi::Device &device) noexcept
    : m_linearSampler(device.create_sampler_descriptor({
          .magFilter = rhi::Filter::linear,
          .minFilter = rhi::Filter::linear,
          .mipmapMode = rhi::SamplerMipmapMode::linear,
          .addressModeU = rhi::SamplerAddressMode::clamp_to_edge,
          .addressModeV = rhi::SamplerAddressMode::clamp_to_edge,
          .addressModeW = rhi::SamplerAddressMode::clamp_to_edge,
      })),
      m_nearestSampler(device.create_sampler_descriptor({
          .magFilter = rhi::Filter::nearest,
          .minFilter = rhi::Filter::nearest,
          .mipmapMode = rhi::SamplerMipmapMode::nearest,
          .addressModeU = rhi::SamplerAddressMode::clamp_to_edge,
          .addressModeV = rhi::SamplerAddressMode::clamp_to_edge,
          .addressModeW = rhi::SamplerAddressMode::clamp_to_edge,
      }))

{}
} // namespace strobe::imgui
