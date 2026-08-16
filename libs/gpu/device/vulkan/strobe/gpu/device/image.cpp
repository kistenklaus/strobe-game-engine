#include "strobe/gpu/device/image.hpp"
#include "strobe/gpu/device/format_utilts.hpp"
#include "strobe/gpu/device/handle.hpp"
#include "strobe/gpu/device/image_aspect_utils.hpp"
#include "strobe/gpu/device/image_impl.hpp"
#include "strobe/gpu/device/image_view_impl.hpp"
#include "strobe/gpu/device/image_view_type_utils.hpp"
#include "strobe/gpu/vulkan/image_view.hpp"

namespace strobe::gpu {

Image::Image(const Image &o) noexcept : m_handle(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<ImageImpl>(m_handle);
  }
}

Image::Image(Image &&o) noexcept
    : m_handle(std::exchange(o.m_handle, nullptr)) {}

Image &Image::operator=(const Image &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<ImageImpl>(o.m_handle);
  }
  unpin_void_handle<ImageImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

Image &Image::operator=(Image &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<ImageImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

Image::~Image() noexcept { unpin_void_handle<ImageImpl>(m_handle); }

ImageType Image::type() const noexcept {
  auto *impl = void_handle_ptr<ImageImpl>(m_handle);
  return impl->type;
}

Format Image::format() const noexcept {
  auto *impl = void_handle_ptr<ImageImpl>(m_handle);
  return impl->format;
}

uvec3 Image::extent() const noexcept {
  auto *impl = void_handle_ptr<ImageImpl>(m_handle);
  return impl->extent;
}

uint32_t Image::mip_levels() const noexcept {
  auto *impl = void_handle_ptr<ImageImpl>(m_handle);
  return impl->mip_levels;
}

uint32_t Image::arrayLayers() const noexcept {
  auto *impl = void_handle_ptr<ImageImpl>(m_handle);
  return impl->arrayLayers;
}

SampleCount Image::samples() const noexcept {
  auto *impl = void_handle_ptr<ImageImpl>(m_handle);
  return impl->samples;
}

ImageView Image::create_view(ImageViewType type, ImageAspect aspect,
                             const ImageViewCreateInfo &createInfo) {
  auto *impl = void_handle_ptr<ImageImpl>(m_handle);

  const Format format =
      createInfo.format == Format::undefined ? impl->format : createInfo.format;

  const uint32_t mipLevelCount =
      createInfo.mipLevelCount == UINT32_MAX
          ? impl->mip_levels - createInfo.baseMipLevel
          : createInfo.mipLevelCount;

  const uint32_t arrayLayerCount =
      createInfo.arrayLayerCount == UINT32_MAX
          ? impl->arrayLayers - createInfo.baseArrayLayer
          : createInfo.arrayLayerCount;

  assert(createInfo.baseMipLevel < impl->mip_levels);
  assert(createInfo.baseArrayLayer < impl->arrayLayers);
  assert(mipLevelCount > 0);
  assert(arrayLayerCount > 0);
  assert(createInfo.baseMipLevel + mipLevelCount <= impl->mip_levels);
  assert(createInfo.baseArrayLayer + arrayLayerCount <= impl->arrayLayers);

  vulkan::ImageView view = vulkan::create_image_view(
      impl->context.get(),
      vulkan::ImageViewInfo{
          .image = impl->image,
          .type = to_vk_image_view_type(type),
          .flags = 0,
          .format = to_vk_format(format),
          .components =
              {
                  .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                  .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                  .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                  .a = VK_COMPONENT_SWIZZLE_IDENTITY,
              },
          .range =
              {
                  .aspectMask = to_vk_image_aspect(aspect),
                  .baseMipLevel = createInfo.baseMipLevel,
                  .levelCount = mipLevelCount,
                  .baseArrayLayer = createInfo.baseArrayLayer,
                  .layerCount = arrayLayerCount,
              },
      });

  return ImageView{make_void_handle<ImageViewImpl>(*this, view, format)};
}

} // namespace strobe::gpu
