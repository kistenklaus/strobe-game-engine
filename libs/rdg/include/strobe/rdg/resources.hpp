#pragma once

#include "strobe/rdg/lifetime.hpp"

#include <strobe/flow/flow.hpp>
#include <strobe/rhi/objects/buffer.hpp>
#include <strobe/rhi/objects/device.hpp>
#include <strobe/rhi/objects/image.hpp>
#include <strobe/rhi/types/buffer_info.hpp>
#include <strobe/rhi/types/image_info.hpp>

#include <memory>

namespace strobe::rdg {

/**
 * \brief Records construction of an RHI buffer with its compiled lifetime.
 * \code{.cpp}
 * flow::Resource<rhi::Buffer>
 * buffer(flow::Resource<rhi::Device> device,
 *        flow::Resource<rhi::BufferInfo> info);
 * \endcode
 *
 * \param device Resource containing the device used for construction.
 * \param info Resource containing the buffer creation information.
 * \return Flow resource representing the constructed buffer handle.
 * \attention 1 The device pointer must remain valid while the program can
 * execute the construction operation.
 */
inline flow::Resource<rhi::Buffer>
buffer(flow::Resource<rhi::Device> device,
       flow::Resource<rhi::BufferInfo> info) {
  auto result = flow::resource<rhi::Buffer>();
  flow::construct(
      result, flow::read(device), flow::read(info),
      [](rhi::Buffer *output, flow::Lifetime lifetime,
         const rhi::Device &device, const rhi::BufferInfo &info) noexcept {
        std::construct_at(
            output, device.create_buffer(info, memory_lifetime(lifetime)));
      });
  return result;
}

/**
 * \brief Records construction of an RHI image with its compiled lifetime.
 * \code{.cpp}
 * flow::Resource<rhi::Image>
 * image(flow::Resource<rhi::Device> device,
 *       flow::Resource<rhi::ImageInfo> info);
 * \endcode
 *
 * \param device Resource containing the device used for construction.
 * \param info Resource containing the image creation information.
 * \return Flow resource representing the constructed image handle.
 * \attention 1 The device pointer must remain valid while the program can
 * execute the construction operation.
 */
inline flow::Resource<rhi::Image> image(flow::Resource<rhi::Device> device,
                                        flow::Resource<rhi::ImageInfo> info) {
  auto result = flow::resource<rhi::Image>();
  flow::construct(
      result, flow::read(device), flow::read(info),
      [](rhi::Image *output, flow::Lifetime lifetime, const rhi::Device &device,
         const rhi::ImageInfo &info) noexcept {
        std::construct_at(output,
                          device.create_image(info, memory_lifetime(lifetime)));
      });
  return result;
}

} // namespace strobe::rdg
