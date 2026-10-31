#pragma once

#include "hdr.hpp"
#include "radiance_buffer.hpp"
#include "strobe/rhi/objects/compute_shader.hpp"
#include "strobe/rhi/objects/sampler_descriptor.hpp"
#include "taa.slang.spv.hpp"

class TAAPass {
public:
  TAAPass(const strobe::rhi::Device &device)
      : m_shader(device.create_compute_shader({
            .spirv = taa_slang_spv,
        })),
        m_sampler(device.create_sampler_descriptor({
            .magFilter = strobe::rhi::Filter::linear,
            .minFilter = strobe::rhi::Filter::linear,
            .mipmapMode = strobe::rhi::SamplerMipmapMode::nearest,
            .addressModeU = strobe::rhi::SamplerAddressMode::clamp_to_edge,
            .addressModeV = strobe::rhi::SamplerAddressMode::clamp_to_edge,
            .addressModeW = strobe::rhi::SamplerAddressMode::clamp_to_edge,
        })) {}

  void dispatch(strobe::rhi::CommandBuffer &cmd, const RBuffer &rbuffer,
                const HDR &hdr) {
    assert(rbuffer.radiance.extent() == hdr.color.extent());

    strobe::uvec2 extent = rbuffer.radiance.extent().xy();
    cmd.bind_shader(m_shader);
    cmd.push(0, &extent, sizeof(extent));
    cmd.push(8, rbuffer.radianceStorage);
    cmd.push(12, hdr.colorStorageDescriptor);

    strobe::uvec2 wgCount = (extent + WG_SIZE - 1) / WG_SIZE;
    cmd.dispatch(wgCount.x(), wgCount.y());
  }

private:
  static constexpr strobe::uvec2 WG_SIZE = {16, 16};
  strobe::rhi::ComputeShader m_shader;
  strobe::rhi::SamplerDescriptor m_sampler;
};
