#pragma once
#include "strobe/ecs/allocator.hpp"
#include "strobe/ecs/event_registry.hpp"
#include "strobe/ecs/resource_registry.hpp"
#include "strobe/ecs/schedule/job_scheduler.hpp"

namespace strobe {

class ECS {
  static constexpr uint32_t THREAD_COUNT = 8;

public:
  ECS(const ecs::allocator &alloc = {})
      : m_alloc(alloc), m_rreg(&m_alloc), m_ereg(&m_alloc),
        m_scheduler(&m_alloc, THREAD_COUNT, 1) {}

  ECS(const ECS &) = delete;
  ECS &operator=(const ECS &) = delete;
  ECS(ECS &&) = delete;
  ECS &operator=(ECS &&) = delete;

private:
  ecs::allocator m_alloc;
  ecs::ResourceRegistry<ecs::allocator_ref> m_rreg;
  ecs::EventRegistry m_ereg;
  ecs::JobScheduler m_scheduler;
};

} // namespace strobe
