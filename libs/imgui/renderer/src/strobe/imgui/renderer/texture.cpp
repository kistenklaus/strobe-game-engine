#include "strobe/imgui/renderer/texture.hpp"
#include "imgui.h"

namespace strobe::imgui::renderer {

Texture *Texture::create(rhi::Device &device, rhi::CommandBuffer cmd,
                         ImTextureData *data) noexcept {
  assert(data);
  assert(data->BackendUserData == nullptr);
  assert(data->GetTexID() == ImTextureID_Invalid);
  assert(data->Status == ImTextureStatus_WantCreate);

  rhi::Format format;
  switch (data->Format) {
  case ImTextureFormat_RGBA32:
    format = rhi::Format::rgba8_unorm;
    break;

  case ImTextureFormat_Alpha8:
    fmt::println("Alpha8 ImGui textures are not supported");
    std::terminate();

  default:
    std::unreachable();
  }

  rhi::Image image = device.create_image({
      .type = rhi::ImageType::image_2d,
      .format = format,
      .extent =
          uvec3{
              static_cast<uint32_t>(data->Width),
              static_cast<uint32_t>(data->Height),
              1,
          },
      .imageUsage = rhi::ImageUsage::sampled | rhi::ImageUsage::transfer_dst,
  });

  rhi::ResourceDescriptor descriptor = device.create_sampled_image_descriptor({
      .image = image,
      .layout = rhi::ImageLayout::read_only,
      .subresource =
          {
              .aspect = rhi::ImageAspect::color,
          },
  });

  auto *texture = new Texture{
      std::move(image),
      std::move(descriptor),
  };

  data->BackendUserData = texture;
  data->SetTexID(
      static_cast<ImTextureID>(reinterpret_cast<uintptr_t>(texture)));

  cmd.transition_image(texture->m_image, rhi::ImageLayout::undefined,
                       rhi::ImageLayout::transfer_dst);

  cmd.update(
      {
          .image = texture->m_image,
          .subresource =
              {
                  .aspect = rhi::ImageAspect::color,
                  .mipLevel = 0,
                  .baseArrayLayer = 0,
                  .layerCount = 1,
              },
          .offset = ivec3{0, 0, 0},
          .extent =
              uvec3{
                  static_cast<uint32_t>(data->Width),
                  static_cast<uint32_t>(data->Height),
                  1,
              },
      },
      data->GetPixels(), static_cast<uint32_t>(data->Width), 0,
      rhi::ImageLayout::transfer_dst);

  cmd.transition_image(texture->m_image, rhi::ImageLayout::transfer_dst,
                       rhi::ImageLayout::read_only);

  data->SetStatus(ImTextureStatus_OK);
  return texture;
}
void Texture::update(rhi::CommandBuffer cmd, ImTextureData *data) noexcept {
  assert(data);
  assert(data->BackendUserData);
  assert(data->Status == ImTextureStatus_WantUpdates);

  auto *texture = static_cast<Texture *>(data->BackendUserData);

  const uint32_t x = static_cast<uint32_t>(data->UpdateRect.x);
  const uint32_t y = static_cast<uint32_t>(data->UpdateRect.y);
  const uint32_t width = static_cast<uint32_t>(data->UpdateRect.w);
  const uint32_t height = static_cast<uint32_t>(data->UpdateRect.h);

  if (width == 0 || height == 0) {
    data->SetStatus(ImTextureStatus_OK);
    return;
  }

  cmd.transition_image(texture->m_image, rhi::ImageLayout::read_only,
                       rhi::ImageLayout::transfer_dst);

  cmd.update(
      {
          .image = texture->m_image,
          .subresource =
              {
                  .aspect = rhi::ImageAspect::color,
                  .mipLevel = 0,
                  .baseArrayLayer = 0,
                  .layerCount = 1,
              },
          .offset = ivec3{static_cast<int32_t>(x), static_cast<int32_t>(y), 0},
          .extent = uvec3{width, height, 1},
      },
      data->GetPixelsAt(x, y),
      // Source rows retain the pitch of the complete ImGui texture.
      static_cast<uint32_t>(data->Width), 0, rhi::ImageLayout::transfer_dst);

  cmd.transition_image(texture->m_image, rhi::ImageLayout::transfer_dst,
                       rhi::ImageLayout::read_only);

  data->SetStatus(ImTextureStatus_OK);
}
void Texture::destroy(ImTextureData *data) noexcept {
  assert(data);
  if (auto *tex = static_cast<Texture *>(data->BackendUserData)) {
    delete tex;
    data->BackendUserData = nullptr;
    data->SetTexID(ImTextureID_Invalid);
  }
  data->SetStatus(ImTextureStatus_Destroyed);
}

Texture::Texture(rhi::Image image, rhi::ResourceDescriptor descriptor) noexcept
    : m_image(std::move(image)), m_descriptor(std::move(descriptor)) {};

void update_textures(rhi::Device &device, rhi::CommandBuffer &cmd,
                     const ImDrawData *drawData) noexcept {
  if (drawData->Textures) {
    for (ImTextureData *data : *drawData->Textures) {
      assert(data);
      switch (data->Status) {
      case ImTextureStatus_OK:
      case ImTextureStatus_Destroyed:
        continue;
      case ImTextureStatus_WantCreate:
        Texture::create(device, cmd, data);
        break;
      case ImTextureStatus_WantUpdates:
        Texture::update(cmd, data);
        break;
      case ImTextureStatus_WantDestroy:
        Texture::destroy(data);
        break;
      }
    }
  }
}
void cleanup_texture() noexcept {
  auto &platform_io = ImGui::GetPlatformIO();
  for (const auto &data : platform_io.Textures) {
    if (auto *tex = static_cast<Texture *>(data->BackendUserData)) {
      delete tex;
      data->BackendUserData = nullptr;
      data->SetTexID(ImTextureID_Invalid);
    }
    data->SetStatus(ImTextureStatus_Destroyed);
  }
}

} // namespace strobe::imgui::renderer
