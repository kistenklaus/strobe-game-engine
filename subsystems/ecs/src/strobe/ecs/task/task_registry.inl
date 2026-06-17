#pragma once

#include "strobe/core/type_traits/types.hpp"
#include "strobe/ecs/object/object_utils.hpp"
#include "strobe/ecs/task/task_registry.hpp"
#include "strobe/ecs/universe.hpp"

namespace strobe::ecs {

template <task_fn Fn> void TaskRegistry::cmd_submit() noexcept {
  using traits = task_traits<Fn>;
  using task_type = typename task_traits<Fn>::task_type;
  constexpr task_curry fn = [](TaskRegistry *treg) noexcept {
    task_header *header = treg->require_task_header<task_type>();
    using lifetime_arguments = typename traits::lifetime_arguments;
    bool skip = false;
    for_each_type<lifetime_arguments>([&]<typename object>() noexcept {
      lifetime_id id = get_object_lifetime<object>(treg->m_universe);
      if (!treg->m_universe->lreg.is_live(id)) {
        skip = true;
      }
    });
    if (skip) {
      // TODO: use proper logging utility.
      fmt::println("WARNING: Skipped task, because arguments are not live");
      return;
    }
    header->submit(treg->scheduler_ptr());
  };
  chunk *current = m_last.load(std::memory_order_acquire);
  const std::uint32_t index =
      current->count.fetch_add(1, std::memory_order_relaxed);
  if (index < chunk_size) {
    current->tasks[index] = fn;
    return;
  }
  chunk *next = m_chunkPool.acquire();
  next->tasks[0] = fn;
  next->count.store(1, std::memory_order_relaxed);
  chunk *previous = m_last.exchange(next, std::memory_order_acq_rel);
  previous->next.store(next, std::memory_order_release);
}

inline Scheduler *TaskRegistry::scheduler_ptr() const noexcept {
  return &m_universe->scheduler;
}

inline location TaskRegistry::tloc() const noexcept { return m_universe->t_location; }

} // namespace strobe::ecs
