#pragma once

#include "strobe/rhi/objects/command_buffer.hpp"
#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/objects/swapchain_image.hpp"
#include "strobe/rhi/objects/timepoint.hpp"
#include "strobe/rhi/types/pipeline_stage.hpp"

    namespace strobe::rhi {

  /**
   * \ingroup rhi
   * \brief Device execution queue.
   * Defined in header <strobe/rhi/rhi.hpp>
   * \code{.cpp}
   * class Queue : public Object<Queue>;
   * \endcode
   *
   * Records queue dependencies, submits command buffers, and presents swapchain
   * images.
   *
   * \attention 1. Host access to a Queue must be externally synchronized.
   */
  class Queue : public Object<Queue> {
    friend class Object<Queue>;
    static void pin(void *) noexcept;
    static void unpin(void *) noexcept;

  public:
    using Object::Object;

    /**
     * \brief Adds queue wait.
     * \code{.cpp}
     * void wait(const Timepoint& timepoint, PipelineState stage = PipelineStage::all_commands) noexcept;   (1)
     * void wait(const SwapchainImage& image, PipelineStage& stage = PipelineStage::all_commands) noexcept; (2)
     * \endcode
     *
     * Adds a dependency on \p timepoint to subsequent queue submissions.
     *
     * \param timepoint Timepoint to wait for.
     * \param stage Earliest pipeline stage affected by the wait.
     *
     * \attention 1. \p timepoint must be valid.
     * \attention 2. The Queue must be externally synchronized.
     */
    void wait(const Timepoint &timepoint,
              PipelineStage stage = PipelineStage::all_commands) noexcept;
    void wait(const SwapchainImage &image,
              PipelineStage stage = PipelineStage::all_commands) noexcept;

    /**
     * \brief Submits command buffers.
     * \code{.cpp}
     * Timepoint submit(span<const CommandBuffer> cmds) noexcept;
     * \endcode
     *
     * Adds \p cmds to the queue together with currently pending waits.
     *
     * \param cmds Command buffers to submit.
     *
     * \return Timepoint at which the submission completes.
     *
     * \attention 1. Every command buffer in \p cmds must have finished
     * recording.
     * \attention 2. Every command buffer must be compatible with this Queue.
     * \attention 3. Submitted command buffers and referenced resources must
     * remain valid until execution completes.
     * \attention 4. The Queue must be externally synchronized.
     */
    Timepoint submit(span<const CommandBuffer> cmds) noexcept;

    /**
     * \brief Presents swapchain image.
     * \code{.cpp}
     * void present(SwapchainImage image) noexcept;
     * \endcode
     *
     * Schedules presentation of \p swapchainImage after preceding queue work.
     *
     * \param image Swapchain image to present.
     *
     * \attention 1. \p swapchainImage must reference an acquired image.
     * \attention 2. The image must be in a valid presentation layout.
     * \attention 3. The image must not have been presented previously.
     * \attention 4. The Queue must support presentation for the image's
     * swapchain surface.
     * \attention 5. The Queue must be externally synchronized.
     */
    void present(SwapchainImage image) noexcept;

    /**
     * \brief Flushes pending work.
     * \code{.cpp}
     * void flush() noexcept;
     * \endcode
     *
     * Submits all queue operations accumulated so far.
     *
     * \attention 1. The Queue must be externally synchronized.
     */
    void flush() noexcept;
  };

} // namespace strobe::rhi
