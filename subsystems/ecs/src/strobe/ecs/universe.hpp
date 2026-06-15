#pragma once

#include "strobe/ecs/allocator.hpp"
#include "strobe/ecs/cmd/drain_cmds.hpp"
#include "strobe/ecs/lifetime/lifetime_registry.hpp"
#include "strobe/ecs/resource/resource_registry.hpp"
#include "strobe/ecs/scheduler/op_schedule.hpp"
#include "strobe/ecs/scheduler/scheduler.hpp"
#include "strobe/ecs/system/system_registry.hpp"
#include "strobe/ecs/task/task_registry.hpp"
#include <thread>

namespace strobe::ecs {

struct Universe {
  TaskRegistry treg;
  LifetimeRegistry lreg;
  Scheduler scheduler;
  location t_location;
  location sr_location;
  cmd_domain sr_domain;
  ResourceRegistry rreg;
  OpSchedule schedule;
  SystemRegistry sreg;

  Universe(const strobe::ecs::allocator_ref alloc, uint32_t threadCount)
      : treg(this, alloc), lreg(alloc), scheduler(alloc, threadCount),
        t_location(scheduler.alloc()), sr_location{scheduler.alloc()},
        sr_domain{}, rreg(this, alloc), schedule{alloc}, sreg(this, alloc) {}

  ~Universe() {
    treg.drain_cmds();
    drain_cmds(&scheduler, &sr_domain, op_scope(acq_rel(sr_location)),
               sreg.cmds(), rreg.cmds());
  }
};

} // namespace strobe::ecs

#include "strobe/ecs/object/object_utils.inl"
#include "strobe/ecs/resource/resource_registry.inl"
#include "strobe/ecs/system/system_blocks.inl"
#include "strobe/ecs/system/system_registry.inl"
#include "strobe/ecs/task/task_registry.inl"
