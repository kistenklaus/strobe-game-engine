#pragma once

#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/objects/timepoint.hpp"
#include "strobe/rhi/sync/timeline_notify_flag.hpp"
#include "strobe/rhi/vulkan/context/context.hpp"
#include "strobe/rhi/vulkan/timeline_semaphore.hpp"

namespace strobe::rhi {

class Timeline : public Object<Timeline> {
  friend class Object<Timeline>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;

public:
  using Object::Object;
  explicit Timeline(void *handle) noexcept : Object(handle) {}

  static void
  notify(const Timepoint &timepoint,
         TimelineNotifyFlag flag = TimelineNotifyFlag::block) noexcept;
  void notify(uint64_t serial,
              TimelineNotifyFlag flag = TimelineNotifyFlag::block) noexcept;

  Timepoint now() noexcept;
  bool contains(Timepoint timepoint) const noexcept;
  void complete(Timepoint timepoint) noexcept;
  Timepoint advance() noexcept;
  uint64_t serial() const noexcept;
  uint64_t completed_serial() const noexcept;
  vulkan::TimelineSemaphore timelineSemaphore() const noexcept;

  void install_commit(void *pUserData,
                      void (*commit)(void *, Timepoint)) noexcept;
  void uninstall_commit() noexcept;

  Timepoint from_serial(uint64_t serial) noexcept;

  vulkan::Context *ctx() const noexcept;
};

} // namespace strobe::rhi
