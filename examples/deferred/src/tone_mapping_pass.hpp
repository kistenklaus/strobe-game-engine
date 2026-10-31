#pragma once

#include "hdr.hpp"
#include "strobe/core/lina/vec.hpp"
#include "strobe/rhi/objects/compute_shader.hpp"
#include "strobe/rhi/objects/image.hpp"
#include "target.hpp"
#include "tone_mapping.slang.spv.hpp"

class ToneMappingPass {
public:
  ToneMappingPass(const strobe::rhi::Device &device)
      : m_shader(device.create_compute_shader({
            .spirv = tone_mapping_slang_spv,
        })) {}

  // writes target reads hdr.
  void dispatch(strobe::rhi::CommandBuffer &cmd, const Target &target,
                const HDR &hdr) noexcept {
    assert(hdr.color.extent() == target.color.extent());

    strobe::uvec2 extent = target.extent();
    cmd.push(0, &extent, sizeof(extent));
    cmd.push(8, hdr.colorStorageDescriptor);
    cmd.push(12, target.colorStorageDescriptor);

    cmd.bind_shader(m_shader);
    strobe::uvec2 wgCount = (extent + WG_SIZE - 1) / WG_SIZE;
    cmd.dispatch(wgCount.x(), wgCount.y());

  }

private:
  static constexpr strobe::uvec2 WG_SIZE = {16, 16};
  strobe::rhi::ComputeShader m_shader;
};
