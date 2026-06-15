#pragma once

#include "strobe/ecs/scheduler/op_scope.hpp"
#include "strobe/ecs/task/task_registry.hpp"
#include "strobe/ecs/task/task_traits.hpp"
#include <type_traits>

namespace strobe {

namespace ecs {

struct task_cmd_tag {
  static constexpr bool is_object = true;
};

} // namespace ecs

class TaskCommands {
public:
  using object_tag = ecs::task_cmd_tag;
  using memory_order = ecs::release_access;

  explicit TaskCommands(ecs::TaskRegistry *treg) : m_treg(treg) {}

  template <ecs::task_fn Fn> void submit(Fn) noexcept {
    m_treg->template cmd_submit<std::remove_cvref_t<Fn>>();
  }

  template <ecs::task_fn Fn> void submit() noexcept {
    m_treg->template cmd_submit<std::remove_cvref_t<Fn>>();
  }

private:
  ecs::TaskRegistry *m_treg;
};

} // namespace strobe
