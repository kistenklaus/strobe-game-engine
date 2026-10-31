#pragma once

#include "strobe/core/lina/vec.hpp"
#include "strobe/rhi/objects/image.hpp"
#include "strobe/rhi/objects/resource_descriptor.hpp"
struct Target {
  strobe::rhi::Image color;
  strobe::rhi::ResourceDescriptor colorStorageDescriptor;

  strobe::uvec2 extent() const noexcept {
    return color.extent().xy();
  }
};

