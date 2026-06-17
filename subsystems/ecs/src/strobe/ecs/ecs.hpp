#pragma once
#include "strobe/ecs/allocator.hpp"
#include "strobe/ecs/cmd/drain_cmds.hpp"
#include "strobe/ecs/scheduler/op_scope.hpp"
#include "strobe/ecs/task/task_traits.hpp"
#include "strobe/ecs/universe.hpp"
#include <algorithm>
#include <thread>

namespace strobe {

class ECS {
public:
  template <ecs::task_fn MainFn>
  explicit ECS(MainFn,
               uint32_t thread_count = std::max<uint32_t>(
                   std::thread::hardware_concurrency() * 3 / 2, 1),
               const ecs::allocator &alloc = {})
      : m_alloc(alloc), m_universe(&m_alloc, thread_count) {
    m_universe.treg.cmd_submit<MainFn>();
    m_submissionThread = std::jthread(
        [this](std::stop_token stop_token) { main_thread(stop_token); });
  }

  ECS(const ECS &) = delete;
  ECS &operator=(const ECS &) = delete;
  ECS(ECS &&) = delete;
  ECS &operator=(ECS &&) = delete;

  void stop() noexcept { m_submissionThread.request_stop(); }

  void join() noexcept {
    m_submissionThread.join();
  }

private:
  void main_thread(std::stop_token stop_token);

  ecs::allocator m_alloc;
  std::jthread m_submissionThread;
  ecs::Universe m_universe;
};

} // namespace strobe
