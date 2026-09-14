#pragma once

#include "strobe/rhi/objects/command_buffer.hpp"
#include "strobe/rhi/types/command_buffer_type.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Allocates command buffers.
 *
 * Defined in header <strobe/rhi/rhi.hpp>
 *
 * \code{.cpp}
 * class CommandPool : public : Object<CommandPool>;
 * \endcode
 *
 * CommandPool allocates and owns the backing storage of CommandBuffer objects.
 *
 * \attention 1. Host access to the CommandPool must be externally synchronized.
 * \attention 2. Recording any CommandBuffer allocated from this pool counts as
 * host access to the pool.
 */
class CommandPool : public Object<CommandPool> {
  friend class Object<CommandPool>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;

public:
  using Object::Object;

  /**
   * \brief Allocates command buffer.
   * \code{.cpp}
   * CommandBuffer alloc(CommandBufferFlags flags = CommandBufferFlags::none) noexcept;
   * \endcode
   *
   * Allocates a CommandBuffer associated with this pool.
   *
   * \param flags Command buffer allocation and usage flags.
   *
   * \return Command buffer allocated from this pool.
   *
   * \attention 1. The CommandPool must be externally synchronized.
   */
  CommandBuffer alloc(CommandBufferFlags flags = CommandBufferFlags::none) noexcept;
};

} // namespace strobe::rhi
