#pragma once

#include "strobe/rhi/types/border_color.hpp"
#include "strobe/rhi/types/compare_op.hpp"
#include "strobe/rhi/types/filter.hpp"
#include "strobe/rhi/types/sampler_address_mode.hpp"
#include "strobe/rhi/types/sampler_mipmap_mode.hpp"
#include <limits>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Sample descriptor create info
 * \snippet{cpp} sampler_descriptor_info.hpp SamplerDescriptorInfo
 */
// [SamplerDescriptorInfo]
struct SamplerDescriptorInfo {
  Filter magFilter = Filter::nearest;
  Filter minFilter = Filter::nearest;
  SamplerMipmapMode mipmapMode = SamplerMipmapMode::nearest;
  SamplerAddressMode addressModeU = SamplerAddressMode::repeat;
  SamplerAddressMode addressModeV = SamplerAddressMode::repeat;
  SamplerAddressMode addressModeW = SamplerAddressMode::repeat;
  float mipLodBias = 0.0f;
  bool anisotropyEnable = false;
  float maxAnisotropy = 1.0f;
  bool compareEnable = false;
  CompareOp compareOp = CompareOp::always;
  float minLod = 0.0f;
  float maxLod = std::numeric_limits<float>::infinity();
  BorderColor borderColor = BorderColor::float_transparent;
  bool unnormalizedCoordinates = false;
};
// [SamplerDescriptorInfo]

} // namespace strobe::rhi
