#pragma once

#include "strobe/rhi/types/access.hpp"
#include "strobe/rhi/types/pipeline_stage.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief todo
 */
struct AccessScope {
  PipelineStage stage;
  Access access;
};

[[nodiscard]] constexpr AccessScope operator|(const AccessScope &lhs,
                                              const AccessScope &rhs) {
  return AccessScope{
      .stage = lhs.stage | rhs.stage,
      .access = lhs.access | rhs.access,
  };
}

namespace access {

/**
 * \ingroup rhi
 * \brief todo
 */
inline constexpr AccessScope none{
    .stage = PipelineStage::none,
    .access = Access::none,
};
/**
 * \ingroup rhi
 * \brief todo
 */
inline constexpr AccessScope index_read{
    .stage = PipelineStage::index_input,
    .access = Access::index_read,
};
/**
 * \ingroup rhi
 * \brief todo
 */
inline constexpr AccessScope vertex_attribute_read = AccessScope{
    .stage = PipelineStage::vertex_attribute_input,
    .access = Access::vertex_attribute_read,
};
/**
 * \ingroup rhi
 * \brief todo
 */
inline constexpr AccessScope transfer_write = AccessScope{
    .stage = PipelineStage::transfer,
    .access = Access::transfer_write,
};
/**
 * \ingroup rhi
 * \brief todo
 */
inline constexpr AccessScope transfer_read = AccessScope{
    .stage = PipelineStage::transfer,
    .access = Access::transfer_read,
};
/**
 * \ingroup rhi
 * \brief todo
 */
inline constexpr AccessScope transfer_read_write =
    transfer_read | transfer_write;

/**
 * \ingroup rhi
 * \brief todo
 */
inline constexpr AccessScope color_attachment_read{
    .stage = PipelineStage::color_attachment_output,
    .access = Access::color_attachment_read,
};

/**
 * \ingroup rhi
 * \brief todo
 */
inline constexpr AccessScope color_attachment_write{
    .stage = PipelineStage::color_attachment_output,
    .access = Access::color_attachment_write,
};

/**
 * \ingroup rhi
 * \brief todo
 */
inline constexpr AccessScope color_attachment_read_write =
    color_attachment_read | color_attachment_write;

/**
 * \ingroup rhi
 * \brief todo
 */
inline constexpr AccessScope depth_stencil_read{
    .stage = PipelineStage::early_fragment_tests |
             PipelineStage::late_fragment_tests,
    .access = Access::depth_stencil_attachment_read,
};

/**
 * \ingroup rhi
 * \brief todo
 */
inline constexpr AccessScope depth_stencil_write{
    .stage = PipelineStage::early_fragment_tests |
             PipelineStage::late_fragment_tests,
    .access = Access::depth_stencil_attachment_write,
};

/**
 * \ingroup rhi
 * \brief todo
 */
inline constexpr AccessScope depth_stencil_read_write =
    depth_stencil_read | depth_stencil_write;

/**
 * \ingroup rhi
 * \brief todo
 */
inline constexpr AccessScope host_read{
    .stage = PipelineStage::host,
    .access = Access::host_read,
};

/**
 * \ingroup rhi
 * \brief todo
 */
inline constexpr AccessScope host_write{
    .stage = PipelineStage::host,
    .access = Access::host_write,
};

/**
 * \ingroup rhi
 * \brief todo
 */
inline constexpr AccessScope host_read_write = host_read | host_write;

} // namespace access

} // namespace strobe::rhi
