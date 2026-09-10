#pragma once

#include "strobe/rhi/objects/command_buffer.hpp"
#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/sync/binary_semaphore.hpp"
#include "strobe/rhi/objects/timepoint.hpp"
#include "strobe/rhi/sync/fence.hpp"

namespace strobe::rhi {

class GarbageCollector : public Object<GarbageCollector> {
  friend class Object<GarbageCollector>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;

public:
  using Object::Object;
  explicit GarbageCollector(void *handle) noexcept : Object(handle) {}

  void request_commit(Timepoint timepoint) noexcept;

  void retire(Timepoint timepoint) noexcept;
  void retire(Timepoint timepoint, span<const CommandBuffer> cmds);
  void retire(Timepoint timepoint, span<const BinarySemaphore> sems);
  void retire(Fence fence) noexcept;

};

} // namespace strobe::rhi
