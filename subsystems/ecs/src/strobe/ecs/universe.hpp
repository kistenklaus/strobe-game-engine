#pragma once

#include "strobe/ecs/allocator.hpp"
#include "strobe/ecs/lifetime/lifetime_registry.hpp"
#include "strobe/ecs/resource/resource_registry.hpp"
#include "strobe/ecs/scheduler/op_schedule.hpp"
#include "strobe/ecs/scheduler/scheduler.hpp"
#include "strobe/ecs/system/system_registry.hpp"

namespace strobe::ecs {

struct Universe {
  Scheduler scheduler;
  LifetimeRegistry lreg;
  cmd_domain sr_domain;
  location sr_location;
  ResourceRegistry rreg;
  OpSchedule schedule;
  SystemRegistry sreg;

  Universe(const strobe::ecs::allocator_ref alloc, uint32_t threadCount)
      : scheduler(alloc, threadCount), lreg(alloc), sr_domain{},
        sr_location{scheduler.alloc()}, rreg(this, alloc), schedule{alloc},
        sreg(this, alloc) {}

  ~Universe()
  {
      scheduler.free(sr_location);
  }
};

} // namespace strobe::ecs

#include "strobe/ecs/object/object_utils.inl"
#include "strobe/ecs/resource/resource_registry.inl"
#include "strobe/ecs/system/system_blocks.inl"
#include "strobe/ecs/system/system_registry.inl"
