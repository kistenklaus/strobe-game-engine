#include "strobe/rhi/objects/image.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/memory/image_handle_alloc.hpp"
#include "strobe/rhi/memory/image_impl.hpp"
#include "strobe/rhi/memory/image_view_impl.hpp"
#include "strobe/rhi/utils/format_utilts.hpp"
#include "strobe/rhi/utils/image_aspect_utils.hpp"
#include "strobe/rhi/utils/image_view_type_utils.hpp"
#include "strobe/rhi/vulkan/image_view.hpp"

namespace strobe::rhi {

using handle_alloc = image_handle_alloc_ref;

void unpin_image(void *h) noexcept {
  if (h == nullptr) {
    return;
  }
  using control_block = handle_control_block<ImageImpl, handle_alloc>;
  using allocator_traits = AllocatorTraits<handle_alloc>;

  auto *block = static_cast<control_block *>(h);

  if (block->refCount.fetch_sub(1, std::memory_order_acq_rel) != 1) {
    return;
  }
  MemoryAllocation keepAlive = block->value.allocation;
  handle_alloc alloc = std::move(block->alloc);
  std::destroy_at(block);
  // alloc, still has to be valid after destroy!, but destroy might destroy the
  // underlying pool!
  allocator_traits::template deallocate<control_block>(alloc, block);
}

Image::Image(const Image &o) noexcept : m_handle(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<ImageImpl, handle_alloc>(m_handle);
  }
}

Image::Image(Image &&o) noexcept
    : m_handle(std::exchange(o.m_handle, nullptr)) {}

Image &Image::operator=(const Image &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<ImageImpl, handle_alloc>(o.m_handle);
  }
  unpin_image(m_handle);
  m_handle = o.m_handle;
  return *this;
}

Image &Image::operator=(Image &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_image(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

Image::~Image() noexcept { unpin_image(m_handle); }

ImageType Image::type() const noexcept {
  auto *impl = void_handle_ptr<ImageImpl, handle_alloc>(m_handle);
  return impl->type;
}

Format Image::format() const noexcept {
  auto *impl = void_handle_ptr<ImageImpl, handle_alloc>(m_handle);
  return impl->format;
}

uvec3 Image::extent() const noexcept {
  auto *impl = void_handle_ptr<ImageImpl, handle_alloc>(m_handle);
  return impl->extent;
}

uint32_t Image::mip_levels() const noexcept {
  auto *impl = void_handle_ptr<ImageImpl, handle_alloc>(m_handle);
  return impl->mip_levels;
}

uint32_t Image::arrayLayers() const noexcept {
  auto *impl = void_handle_ptr<ImageImpl, handle_alloc>(m_handle);
  return impl->arrayLayers;
}

SampleCount Image::samples() const noexcept {
  auto *impl = void_handle_ptr<ImageImpl, handle_alloc>(m_handle);
  return impl->samples;
}

ImageView Image::create_view(ImageViewType type, ImageAspect aspect,
                             const ImageViewCreateInfo &createInfo) {
  auto *impl = void_handle_ptr<ImageImpl, handle_alloc>(m_handle);

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
      impl->context.ctx(),
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

} // namespace strobe::rhi
