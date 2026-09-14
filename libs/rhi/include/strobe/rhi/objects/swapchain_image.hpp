#pragma once

#include "strobe/rhi/objects/image.hpp"
#include "strobe/rhi/objects/image_view.hpp"
#include "strobe/rhi/objects/object.hpp"

    namespace strobe::rhi {

  /**
   * \ingroup rhi
   * \brief Swapchain image handle.
   * Defined in header <strobe/rhi/rhi.hpp>
   * \code{.cpp}
   * class SwapchainImage : public Object<SwapchainImage>;
   * \endcode
   *
   * Represents an image acquired from a presentation swapchain together with
   * its associated image view.
   */
  class SwapchainImage : public Object<SwapchainImage> {
    friend class Object<SwapchainImage>;
    static void pin(void *) noexcept;
    static void unpin(void *) noexcept;

  public:
    using Object::Object;

    /**
     * \brief Gets swapchain image.
     * \code{.cpp}
     * const Image& image() const noexcept;
     * \endcode
     *
     * Returns the underlying image.
     *
     * \return Swapchain image.
     */
    const Image &image() const noexcept;

    /**
     * \brief Gets image view.
     * \code{.cpp}
     * const ImageView& view() const noexcept;
     * \endcode
     *
     * Returns the image view associated with the swapchain image.
     *
     * \return Swapchain image view.
     */
    const ImageView &view() const noexcept;

    /**
     * \brief Gets image extent.
     * \code{.cpp}
     * const uvec2 extent() const noexcept;
     * \endcode
     *
     * Returns the extent of the swapchain image.
     *
     * \return Swapchain image extent.
     */
    const uvec2 extent() const noexcept;
  };

} // namespace strobe::rhi
