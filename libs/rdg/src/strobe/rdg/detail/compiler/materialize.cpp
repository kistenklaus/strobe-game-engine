#include "compiler.hpp"
#include "materializer.hpp"
#include "util.hpp"

#include "strobe/rhi/objects/command_buffer.hpp"
#include "strobe/rhi/objects/command_pool.hpp"
#include "strobe/rhi/objects/device.hpp"
#include "strobe/rhi/objects/queue.hpp"

#include <memory>

namespace strobe::rdg::detail {

void materialize_operations(flow::detail::compiler::NormalizedIR &ir,
                            flow::CompileContext &context) {
  const auto &globals = compiler_state(context);
  IntrinsicMaterializer pass(ir, context, "strobe.rdg", 1);

  pass.define("create_cmdpool")
      .metadata("create command pool")
      .arg_nonnull<rhi::CommandPool>(flow::AccessMode::write)
      .global<rhi::Device>(globals.device, flow::AccessMode::read_write)
      .body([](rhi::CommandPool *pool, rhi::Device &device) noexcept {
        std::construct_at(pool, device.create_cmdpool());
      });

  pass.define("submit")
      .metadata("submit command buffer", flow::OpSemantics::side_effect)
      .arg_nonnull<rhi::CommandBuffer>(flow::AccessMode::consume)
      .serialize<rhi::CommandPool>()
      .global<rhi::Queue>(globals.queue, flow::AccessMode::read_write)
      .body([](rhi::CommandBuffer &cmd, rhi::Queue &queue) noexcept {
        queue.submit(span<const rhi::CommandBuffer>(&cmd, 1));
      });

  pass.define("cmd_begin_rendering")
      .metadata("cmd_begin_rendering", flow::OpSemantics::side_effect)
      .immediate<std::uint32_t>()
      .immediate<std::uint32_t>()
      .arg_nonnull<rhi::CommandBuffer>(flow::AccessMode::read_write)
      .serialize<rhi::CommandPool>()
      .body([](std::uint32_t layout_count, std::uint32_t view_mask,
               rhi::CommandBuffer &cmd) noexcept {
        rhi::RenderingInfo rhiInfo;
        rhiInfo.layerCount = layout_count;
        rhiInfo.viewMask = view_mask;
        // ???
        cmd.begin_rendering(rhiInfo);
      });

  pass.define("cmd_end_rendering")
    .metadata("cmd_end_rendering", flow::OpSemantics::side_effect)
    .arg_nonnull<rhi::CommandBuffer>(flow::AccessMode::read_write)
    .serialize<rhi::CommandPool>()
    .body([](rhi::CommandBuffer& cmd) noexcept {
      cmd.end_rendering();
        });


  pass.run();
}

} // namespace strobe::rdg::detail
