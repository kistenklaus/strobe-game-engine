#pragma once

#include "strobe/ecs/task/task_registry.hpp"
#include "strobe/ecs/universe.hpp"

namespace strobe::ecs {

Scheduler *TaskRegistry::scheduler_ptr() const noexcept {
  return &m_universe->scheduler;
}

location TaskRegistry::tloc() const noexcept { return m_universe->t_location; }

} // namespace strobe::ecs
