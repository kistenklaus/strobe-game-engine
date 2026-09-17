#pragma once

#include "imgui.h"
#include "strobe/rhi/objects/command_buffer.hpp"
#include "strobe/rhi/objects/device.hpp"
#include "strobe/rhi/objects/image_view.hpp"
#include "strobe/rhi/types/image_layout.hpp"

namespace strobe::imgui {

class Renderer {
public:
  explicit Renderer(rhi::Device device, uint32_t framesInFlight) noexcept;
  Renderer(const Renderer &) = delete;
  Renderer(Renderer &&o) noexcept;
  Renderer &operator=(const Renderer &) = delete;
  Renderer &operator=(Renderer &&o) noexcept;
  ~Renderer() noexcept;

  void
  render(rhi::CommandBuffer cmd, const ImDrawData *data, rhi::ImageView target,
         rhi::ImageLayout initalLayout = rhi::ImageLayout::undefined,
         rhi::ImageLayout finalLayout = rhi::ImageLayout::present) noexcept;

private:
  void *m_internals = nullptr;
};

} // namespace strobe::imgui
