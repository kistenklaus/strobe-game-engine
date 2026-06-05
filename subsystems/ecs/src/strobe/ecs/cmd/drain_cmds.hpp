#pragma once

#include "strobe/ecs/cmd/cmd_traits.hpp"
#include "strobe/ecs/scheduler/scheduler.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <tuple>
#include <utility>

namespace strobe::ecs {

namespace details {

template <typename Tuple, std::size_t... Is>
inline void assert_same_domain_impl([[maybe_unused]] const Tuple &cmds,
                                    [[maybe_unused]] const cmd_domain *domain,
                                    std::index_sequence<Is...>) noexcept {
  assert(domain != nullptr);
#ifndef NDEBUG
  ((assert(std::get<Is>(cmds)->domain() == domain)), ...);
#endif
}

template <typename Tuple>
inline void assert_same_domain(const Tuple &cmds,
                               const cmd_domain *domain) noexcept {
  constexpr std::size_t count = std::tuple_size_v<Tuple>;
  assert_same_domain_impl(cmds, domain, std::make_index_sequence<count>{});
}

template <typename Tuple, std::size_t N, std::size_t... Is>
inline void refresh_all_impl(const Tuple &cmds, std::array<cmd_index, N> &heads,
                             std::index_sequence<Is...>) noexcept {
  ((heads[Is] = std::get<Is>(cmds)->peek()), ...);
}

template <typename Tuple, std::size_t N>
inline void refresh_all(const Tuple &cmds,
                        std::array<cmd_index, N> &heads) noexcept {
  refresh_all_impl(cmds, heads, std::make_index_sequence<N>{});
}

template <std::size_t N, std::size_t... Is>
inline std::size_t select_min_impl(const std::array<cmd_index, N> &heads,
                                   std::index_sequence<Is...>) noexcept {
  std::size_t best = N;
  cmd_index best_index{null_cmd_index};
  auto consider = [&]<std::size_t I>() noexcept {
    const cmd_index index = heads[I];
    if (index == null_cmd_index) {
      return;
    }
    if (best == N || index < best_index) {
      best = I;
      best_index = index;
    }
  };
  (consider.template operator()<Is>(), ...);
  return best;
}

template <std::size_t N>
inline std::size_t select_min(const std::array<cmd_index, N> &heads) noexcept {
  return select_min_impl(heads, std::make_index_sequence<N>{});
}

template <std::size_t I = 0, typename Tuple>
inline void step_at(const Tuple &cmds, std::size_t index) noexcept {
  if constexpr (I < std::tuple_size_v<Tuple>) {
    if (index == I) {
      std::get<I>(cmds)->step();
    } else {
      step_at<I + 1>(cmds, index);
    }
  }
}

template <std::size_t I = 0, typename Tuple, std::size_t N>
inline void refresh_at(const Tuple &cmds, std::array<cmd_index, N> &heads,
                       std::size_t index) noexcept {
  if constexpr (I < std::tuple_size_v<Tuple>) {
    if (index == I) {
      heads[I] = std::get<I>(cmds)->peek();
    } else {
      refresh_at<I + 1>(cmds, heads, index);
    }
  }
}

template <typename Tuple, std::size_t N>
inline bool drain_visible(const Tuple &cmds,
                          std::array<cmd_index, N> &heads) noexcept {
  bool progressed = false;
  while (true) {
    const std::size_t selected = select_min(heads);
    if (selected == N) {
      return progressed;
    }
    step_at(cmds, selected);
    refresh_at(cmds, heads, selected);
    progressed = true;
  }
}

} // namespace details

template <typename fence_scope, command_buffer... CmdBufs>
void drain_cmds(Scheduler *scheduler, [[maybe_unused]] cmd_domain *domain,
                fence_scope fence, CmdBufs *...cmds) noexcept {
  static_assert(sizeof...(CmdBufs) > 0);
  assert(scheduler != nullptr);
  assert(domain != nullptr);
  constexpr std::size_t count = sizeof...(CmdBufs);
  const auto cmd_tuple = std::tuple<CmdBufs *...>{cmds...};
  details::assert_same_domain(cmd_tuple, domain);
  std::array<cmd_index, count> heads;
  details::refresh_all(cmd_tuple, heads);
  while (true) {
    details::drain_visible(cmd_tuple, heads);
    scheduler->fence(fence);
    details::refresh_all(cmd_tuple, heads);
    const std::size_t selected = details::select_min(heads);
    if (selected == count) {
      domain->reset();
      return;
    }
  }
}

} // namespace strobe::ecs
