#pragma once

#include "strobe/rhi/objects/command_buffer.hpp"
#include "strobe/rhi/types/command_buffer_type.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief todo
 */
class CommandPool : public Object<CommandPool> {
  friend class Object<CommandPool>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;

public:
  using Object::Object;
  CommandBuffer alloc(CommandBufferFlags flags = CommandBufferFlags::none);
};
} // namespace strobe::rhi
