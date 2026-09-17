#pragma once

#include "imgui.h"
#include "strobe/rhi/objects/device.hpp"
#include "strobe/rhi/objects/image.hpp"
#include "strobe/rhi/objects/resource_descriptor.hpp"
#include <fmt/base.h>
#include <fmt/ostream.h>
namespace strobe::imgui {

struct Texture {
public:
  Texture(const Texture &) = delete;
  Texture(Texture &&) = delete;
  static Texture *create(rhi::Device &device, rhi::CommandBuffer cmd,
                         ImTextureData *data) noexcept;

  static void update(rhi::CommandBuffer cmd, ImTextureData *data) noexcept;

  static void destroy(ImTextureData *data) noexcept;

  static Texture *from_id(ImTextureID id) {
    assert(id != ImTextureID_Invalid);
    auto *tex = reinterpret_cast<Texture *>(static_cast<uintptr_t>(id));
    assert(tex);
    return tex;
  }

  const rhi::ResourceDescriptor &descriptor() const noexcept {
    return m_descriptor;
  }

private:
  explicit Texture(rhi::Image image,
                   rhi::ResourceDescriptor descriptor) noexcept;

  rhi::Image m_image;
  rhi::ResourceDescriptor m_descriptor;
};

void update_textures(rhi::Device &device, rhi::CommandBuffer &cmd,
                     const ImDrawData *drawData) noexcept;

} // namespace strobe::imgui
