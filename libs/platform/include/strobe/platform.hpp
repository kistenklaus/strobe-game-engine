#pragma once

#include "strobe/platform/detail.hpp"

#include <memory>
#include <type_traits>

namespace strobe::platform {

/**
 * \ingroup platform
 * \brief Synchronously executes a function on the platform thread.
 *
 * \code{.cpp}
 * template <typename Fn>
 *   requires std::is_invocable_v<Fn &&>
 * auto run(Fn &&fn) noexcept -> std::invoke_result_t<Fn &&>;
 * \endcode
 *
 * Executes \p fn on the platform thread and returns after its execution has
 * completed.
 *
 * \param fn
 *   Function to execute. Exceptions must not escape from the function.
 *
 * \return
 *   The result produced by \p fn. A value result must be movable or copyable.
 *   A reference result must remain valid after run() returns.
 *
 * \attention 1. run() must not be called during the initialization or
 *               destruction of non-local objects with static storage duration.
 */
template <typename Fn>
  requires std::is_invocable_v<Fn &&>
auto run(Fn &&fn) noexcept -> std::invoke_result_t<Fn &&> {
  using Result = std::invoke_result_t<Fn &&>;
  using Context = detail::CallableContext<Fn, Result>;

  Context context{
      std::addressof(fn),
  };

  detail::run(&detail::execute_callable<Fn, Result>, &context);

  return context.result.take();
}

} // namespace strobe::platform
