#pragma once

#include "strobe/ecs/system/system_blocks.hpp"
#include "strobe/ecs/system/system_update_traits.hpp"

namespace strobe::ecs {

template <typename S> void system_block<S>::enter(Universe *universe) noexcept {
  m_start.submit(&universe->scheduler);
  if constexpr (system_update_exists_v<system_type>) {
    universe->schedule.enable(m_op);
    assert(m_op != null_schedule_op);
  }
}

template <typename S> void system_block<S>::exit(Universe *universe) noexcept {
  if constexpr (system_update_exists_v<system_type>) {
    assert(m_op != null_schedule_op);
    universe->schedule.disable(m_op);
  }
  m_stop.submit(&universe->scheduler);
}

} // namespace strobe::ecs
