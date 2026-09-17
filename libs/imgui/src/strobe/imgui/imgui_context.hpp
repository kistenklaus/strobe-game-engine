#pragma once

#include "strobe/core/containers/vector.hpp"
#include "strobe/imgui/imgui_frame.hpp"
#include "strobe/imgui/imgui_samplers.hpp"
#include "strobe/imgui/imgui_shaders.hpp"

namespace strobe::imgui {

struct Context {
  rhi::Device device;
  Vector<Frame> frames;
  Samplers samplers;
  Shaders shaders;
  uint32_t frameIndex;

  explicit Context(rhi::Device device, uint32_t framesInFlight) noexcept
      : device(std::move(device)), frames(static_cast<size_t>(framesInFlight)),
        samplers(this->device), shaders(this->device),frameIndex(0) {}
};

extern Context *g_context;

} // namespace strobe::imgui
