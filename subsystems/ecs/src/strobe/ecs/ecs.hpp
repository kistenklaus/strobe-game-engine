#pragma once
#include "strobe/ecs/allocator.hpp"
#include "strobe/ecs/task/task_traits.hpp"
#include "strobe/ecs/universe.hpp"
#include <algorithm>

namespace strobe {

class ECS {
public:
  template <ecs::task_fn MainFn>
  explicit ECS(MainFn,
      uint32_t thread_count =
          std::max<uint32_t>(std::thread::hardware_concurrency() * 3 / 2, 1),
      const ecs::allocator &alloc = {})
      : m_alloc(alloc), m_universe(&m_alloc, thread_count) {
    m_universe.treg.cmd_submit<MainFn>();
  }

  ECS(const ECS &) = delete;
  ECS &operator=(const ECS &) = delete;
  ECS(ECS &&) = delete;
  ECS &operator=(ECS &&) = delete;

private:
  ecs::allocator m_alloc;
  ecs::Universe m_universe;
};

} // namespace strobe
