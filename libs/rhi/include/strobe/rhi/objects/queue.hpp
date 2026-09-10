#pragma once

#include "strobe/rhi/objects/command_buffer.hpp"
#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/objects/swapchain_image.hpp"
#include "strobe/rhi/objects/timepoint.hpp"
#include "strobe/rhi/types/pipeline_stage.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief todo
 */
class Queue : public Object<Queue> {
public:
  explicit Queue(void *handle) noexcept : Object(handle) {}
  Queue() noexcept : Object(nullptr) {}
  Queue(const Queue &) noexcept;
  Queue(Queue &&) noexcept;
  Queue &operator=(const Queue &) noexcept;
  Queue &operator=(Queue &&) noexcept;
  ~Queue() noexcept;

  void wait(const Timepoint& timepoint,
            PipelineStage stage = PipelineStage::all_commands) noexcept;

  void wait(const SwapchainImage &swapchainImage,
            PipelineStage stage = PipelineStage::all_commands) noexcept;

  Timepoint submit(span<const CommandBuffer> cmds) noexcept;

  void present(SwapchainImage swapchainImage) noexcept;

  void flush() noexcept;
};

} // namespace strobe::rhi
