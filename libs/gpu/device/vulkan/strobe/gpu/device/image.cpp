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
  unpin_void_handle<ImageImpl>(o.m_handle);
  m_handle = o.m_handle;
  return *this;
}

Image &Image::operator=(Image &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<ImageImpl>(o.m_handle);
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

ImageView Image::create_view(const ImageViewCreateInfo &createInfo) {
  auto *impl = void_handle_ptr<ImageImpl>(m_handle);
  auto *device_impl = void_handle_ptr<DeviceImpl>(impl->device.m_handle);

  assert(image);
  assert(image_impl != nullptr);

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

  ImageAspect aspect = createInfo.aspect;

  if (aspect == ImageAspect::automatic) {
    switch (format) {
    case Format::d16_unorm:
    case Format::d32_float:
      aspect = ImageAspect::depth;
      break;

    case Format::d24_unorm_s8_uint:
    case Format::d32_float_s8_uint:
      aspect = ImageAspect::depth_stencil;
      break;

    default:
      aspect = ImageAspect::color;
      break;
    }
  }

  ImageViewType type = createInfo.type;

  if (type == ImageViewType::automatic) {
    switch (impl->type) {
    case ImageType::image_1d:
      type = arrayLayerCount > 1 ? ImageViewType::image_1d_array
                                 : ImageViewType::image_1d;
      break;

    case ImageType::image_2d:
      type = arrayLayerCount > 1 ? ImageViewType::image_2d_array
                                 : ImageViewType::image_2d;
      break;

    case ImageType::image_3d:
      type = ImageViewType::image_3d;
      break;
    }
  }

  assert(createInfo.baseMipLevel < image_impl->mip_levels);
  assert(createInfo.baseArrayLayer < image_impl->arrayLayers);
  assert(mipLevelCount > 0);
  assert(arrayLayerCount > 0);
  assert(createInfo.baseMipLevel + mipLevelCount <= image_impl->mip_levels);
  assert(createInfo.baseArrayLayer + arrayLayerCount <=
         image_impl->arrayLayers);

  vulkan::ImageView view = vulkan::create_image_view(
      &device_impl->context,
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
