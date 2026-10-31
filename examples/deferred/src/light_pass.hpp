#pragma once

#include "radiance_buffer.hpp"
#include "strobe/rhi/objects/device.hpp"
#include "strobe/rhi/objects/fragment_shader.hpp"
#include "strobe/rhi/objects/vertex_shader.hpp"
#include "vbuffer.hpp"
class LightPass {
public:
  LightPass(const strobe::rhi::Device &device) noexcept
      : m_vertexShader(device.create_vertex_shader({
            .spirv = {},
            .nextStage = strobe::rhi::ShaderStage::fragment,
        })),
        m_fragmentShader(device.create_fragment_shader({
            .spirv = {},
        })) {}

  void render(strobe::rhi::CommandBuffer &cmd, const VBuffer &vbuffer,
              const RBuffer &rbuffer) {
    assert(vbuffer.depth.extent() == rbuffer.radiance.extent());
    strobe::rhi::Attachment radianceAtt{
        .view = rbuffer.radianceView,
        .layout = strobe::rhi::ImageLayout::attachment,
        .loadOp = strobe::rhi::AttachmentLoadOp::clear,
    };

    strobe::rhi::Attachment depthAtt{
        .view = vbuffer.depthView,
        .layout = strobe::rhi::ImageLayout::read_only,
        .loadOp = strobe::rhi::AttachmentLoadOp::load,
        .storeOp = strobe::rhi::AttachmentStoreOp::none,
    };

    cmd.begin_rendering({
        .colorAttachments = &radianceAtt,
        .depthAttachment = depthAtt,
    });

    cmd.bind_shader(m_vertexShader);
    cmd.bind_shader(m_fragmentShader);

    strobe::uvec2 extent = rbuffer.radiance.extent().xy();
    cmd.push(0, &extent, sizeof(extent));
    cmd.push(8, vbuffer.depthStorage);
    cmd.push(12, vbuffer.visibilityStorage);
    // camera inverse view projection <- to determine depth!
    // camera view
    
    // storage buffer which contains geometry.
    // storage buffer which contains materials.
    // storage buffer which contains lights.

    // draw lights probably something instanced.

    cmd.end_rendering();
  }

private:
  strobe::rhi::VertexShader m_vertexShader;
  strobe::rhi::FragmentShader m_fragmentShader;
};
