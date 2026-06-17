#pragma once

#include "strobe/ecs/scheduler/op_schedule.hpp"
#include "strobe/ecs/scheduler/operation.hpp"
#include "strobe/ecs/scheduler/scheduler.hpp"
#include "strobe/ecs/system/system_setup_traits.hpp"
#include "strobe/ecs/system/system_start_traits.hpp"
#include "strobe/ecs/system/system_stop_traits.hpp"
#include "strobe/ecs/system/system_teardown_traits.hpp"
#include "strobe/ecs/system/system_traits.hpp"
#include "strobe/ecs/system/system_update_traits.hpp"
#include "strobe/ecs/universe.hpp"
#include <memory>
#include <type_traits>

namespace strobe::ecs {

namespace details {

template <typename S>
inline constexpr auto system_setup_name =
    system_traits<std::remove_cvref_t<S>>::name + fixed_string{"::setup"};

template <typename S>
inline constexpr auto system_start_name =
    system_traits<std::remove_cvref_t<S>>::name + fixed_string{"::start"};

template <typename S>
inline constexpr auto system_update_name =
    system_traits<std::remove_cvref_t<S>>::name + fixed_string{"::update"};

template <typename S>
inline constexpr auto system_stop_name =
    system_traits<std::remove_cvref_t<S>>::name + fixed_string{"::stop"};

template <typename S>
inline constexpr auto system_teardown_name =
    system_traits<std::remove_cvref_t<S>>::name + fixed_string{"::teardown"};

} // namespace details

template <typename S, bool Exists = system_setup_exists_v<S>>
struct system_setup_block;

template <typename S> struct system_setup_block<S, false> {
  using system_type = std::remove_cvref_t<S>;
  explicit system_setup_block(location, Universe *) noexcept {}
  void submit(Scheduler *) noexcept {}
};

template <typename S>
struct system_setup_block<S, true>
    : sync_stateless_object_function<&std::remove_cvref_t<S>::setup,
                                     details::system_setup_name<S>> {
  using system_type = std::remove_cvref_t<S>;
  inline static constexpr auto name = details::system_setup_name<system_type>;
  using base = sync_stateless_object_function<&system_type::setup, name>;
  explicit system_setup_block(location loc, Universe *universe) noexcept
      : base(loc, universe) {}
};

template <typename S>
struct system_setup_block<stateful_system<S>, true>
    : sync_stateful_object_function<&std::remove_cvref_t<S>::setup,
                                    details::system_setup_name<S>> {
  using state_type = std::remove_cvref_t<S>;
  using system_type = stateful_system<state_type>;
  inline static constexpr auto name = details::system_setup_name<system_type>;
  using base = sync_stateful_object_function<&state_type::setup, name>;
  explicit system_setup_block(location loc, Universe *universe) noexcept
      : base(loc, universe) {}
};

template <typename S, bool Exists = system_start_exists_v<S>>
struct system_start_block;

template <typename S> struct system_start_block<S, false> {
  using system_type = std::remove_cvref_t<S>;
  explicit system_start_block(location, Universe *) noexcept {}
  void submit(Scheduler *) noexcept {}
};

template <typename S>
struct system_start_block<S, true>
    : sync_stateless_object_function<&std::remove_cvref_t<S>::start,
                                     details::system_start_name<S>> {
  using system_type = std::remove_cvref_t<S>;
  inline static constexpr auto name = details::system_start_name<system_type>;
  using base = sync_stateless_object_function<&system_type::start, name>;
  explicit system_start_block(location loc, Universe *regs) noexcept
      : base(loc, regs) {}
};

template <typename S>
struct system_start_block<stateful_system<S>, true>
    : sync_stateful_object_function<&std::remove_cvref_t<S>::start,
                                    details::system_start_name<S>> {
  using state_type = std::remove_cvref_t<S>;
  using system_type = stateful_system<state_type>;
  inline static constexpr auto name = details::system_start_name<system_type>;
  using base = sync_stateful_object_function<&state_type::start, name>;
  explicit system_start_block(location loc, Universe *universe) noexcept
      : base(loc, universe) {}
};

template <typename S, bool Exists = system_update_exists_v<S>>
struct system_update_block;

template <typename S> struct system_update_block<S, false> {
  using system_type = std::remove_cvref_t<S>;
  explicit system_update_block(location, Universe *) noexcept {}
  void submit(Scheduler *) noexcept {}
};

template <typename S>
struct system_update_block<S, true>
    : sync_stateless_object_function<&std::remove_cvref_t<S>::update,
                                     details::system_update_name<S>> {
  using system_type = std::remove_cvref_t<S>;
  inline static constexpr auto name = details::system_update_name<system_type>;
  using base = sync_stateless_object_function<&system_type::update, name>;
  explicit system_update_block(location loc, Universe *regs) noexcept
      : base(loc, regs) {}
};

template <typename S>
struct system_update_block<stateful_system<S>, true>
    : sync_stateful_object_function<&std::remove_cvref_t<S>::update,
                                    details::system_update_name<S>> {
  using state_type = std::remove_cvref_t<S>;
  using system_type = stateful_system<state_type>;
  inline static constexpr auto name = details::system_update_name<system_type>;
  using base = sync_stateful_object_function<&state_type::update, name>;
  explicit system_update_block(location loc, Universe *universe) noexcept
      : base(loc, universe) {}
};

template <typename S, bool Exists = system_stop_exists_v<S>>
struct system_stop_block;

template <typename S> struct system_stop_block<S, false> {
  using system_type = std::remove_cvref_t<S>;
  explicit system_stop_block(location, Universe *) noexcept {}
  void submit(Scheduler *) noexcept {}
};

template <typename S>
struct system_stop_block<S, true>
    : sync_stateless_object_function<&std::remove_cvref_t<S>::stop,
                                     details::system_stop_name<S>> {
  using system_type = std::remove_cvref_t<S>;
  inline static constexpr auto name = details::system_stop_name<system_type>;
  using base = sync_stateless_object_function<&system_type::stop, name>;
  explicit system_stop_block(location loc, Universe *regs) noexcept
      : base(loc, regs) {}
};

template <typename S>
struct system_stop_block<stateful_system<S>, true>
    : sync_stateful_object_function<&std::remove_cvref_t<S>::stop,
                                    details::system_stop_name<S>> {
  using state_type = std::remove_cvref_t<S>;
  using system_type = stateful_system<state_type>;
  inline static constexpr auto name = details::system_stop_name<system_type>;
  using base = sync_stateful_object_function<&state_type::stop, name>;
  explicit system_stop_block(location loc, Universe *universe) noexcept
      : base(loc, universe) {}
};

template <typename S, bool Exists = system_teardown_exists_v<S>>
struct system_teardown_block;

template <typename S> struct system_teardown_block<S, false> {
  using system_type = std::remove_cvref_t<S>;
  explicit system_teardown_block(location, Universe *) noexcept {}
  void submit(Scheduler *) noexcept {}
};

template <typename S>
struct system_teardown_block<S, true>
    : sync_stateless_object_function<&std::remove_cvref_t<S>::teardown,
                                     details::system_teardown_name<S>> {
  using system_type = std::remove_cvref_t<S>;
  inline static constexpr auto name =
      details::system_teardown_name<system_type>;
  using base = sync_stateless_object_function<&system_type::teardown, name>;
  explicit system_teardown_block(location loc, Universe *regs) noexcept
      : base(loc, regs) {}
};

template <typename S>
struct system_teardown_block<stateful_system<S>, true>
    : sync_stateful_object_function<&std::remove_cvref_t<S>::teardown,
                                    details::system_teardown_name<S>> {
  using state_type = std::remove_cvref_t<S>;
  using system_type = stateful_system<state_type>;
  inline static constexpr auto name =
      details::system_teardown_name<system_type>;
  using base = sync_stateful_object_function<&state_type::teardown, name>;
  explicit system_teardown_block(location loc, Universe *universe) noexcept
      : base(loc, universe) {}
};

struct system_header {
  explicit system_header(location loc, lifetime_id ready_lifetime,
                         lifetime_id active_lifetime) noexcept
      : m_location(loc), m_ready_lifetime(ready_lifetime),
        m_active_lifetime(active_lifetime) {
    assert(m_location != null_location);
    assert(m_ready_lifetime != null_lifetime_id);
    assert(m_active_lifetime != null_lifetime_id);
  }

  system_header(const system_header &) = delete;
  system_header &operator=(const system_header &) = delete;
  system_header(system_header &&) = delete;
  system_header &operator=(system_header &&) = delete;

  virtual size_t size() const noexcept = 0;
  virtual size_t alignment() const noexcept = 0;

  virtual schedule_op op() const noexcept = 0;

  virtual ~system_header() = default;

  location require_location(Scheduler *scheduler) noexcept {
    if (m_location == null_location) {
      m_location = scheduler->alloc();
    }
    return m_location;
  }
  location get_location() const noexcept {
    assert(m_location != null_location);
    return m_location;
  }
  lifetime_id get_ready_lifetime() const noexcept {
    assert(m_ready_lifetime != null_lifetime_id);
    return m_ready_lifetime;
  }
  lifetime_id get_active_lifetime() const noexcept {
    assert(m_active_lifetime != null_lifetime_id);
    return m_active_lifetime;
  }

private:
  location m_location;
  lifetime_id m_ready_lifetime;
  lifetime_id m_active_lifetime;
};

template <typename system_type>
  requires(!std::is_reference_v<system_type>)
struct system_ready_lifetime_hook : lifetime_hook {
  using setup_storage = system_setup_block<system_type>;
  using teardown_storage = system_teardown_block<system_type>;
  // NOTE: slightly slower dispatch, path, which is fine
  // because ready, lifetime will change rarely.

  explicit system_ready_lifetime_hook(location loc, lifetime_id lifetime,
                                      Universe *universe)
      : m_setup(loc, universe), m_teardown(loc, universe) {
    universe->lreg.install(lifetime, this);
  }

  void enter(Universe *universe) noexcept final override {
    m_setup.submit(&universe->scheduler);
  }

  void exit(Universe *universe) noexcept final override {
    m_teardown.submit(&universe->scheduler);
  }

  [[no_unique_address]] setup_storage m_setup;
  [[no_unique_address]] teardown_storage m_teardown;
};

template <typename S>
struct system_block final : system_header, submit_hook, lifetime_hook {
  using system_type = std::remove_cvref_t<S>;
  using start_storage = system_start_block<system_type>;
  using update_storage = system_update_block<system_type>;
  using stop_storage = system_stop_block<system_type>;

  explicit system_block(location loc, lifetime_id ready_lifetime,
                        lifetime_id active_lifetime,
                        Universe *universe) noexcept
      : system_header(loc, ready_lifetime, active_lifetime),
        m_start(loc, universe), m_update(loc, universe), m_stop(loc, universe),
        m_ready_lifetime_hook{loc, ready_lifetime, universe},
        m_op(system_update_exists_v<system_type>
                 ? universe->schedule.create(this)
                 : null_schedule_op) {
    universe->lreg.install(active_lifetime, this);
  }

  system_block(const system_block &) = delete;
  system_block &operator=(const system_block &) = delete;
  system_block(system_block &&) = delete;
  system_block &operator=(system_block &&) = delete;

  // overrides submit_hook!
  void submit(Scheduler *scheduler) noexcept override {
    m_update.submit(scheduler);
  }

  // active enter (lifetime_hook)
  void enter(Universe *universe) noexcept override;

  // active exit (lifetime_hook)
  void exit(Universe *universe) noexcept override;

  size_t size() const noexcept override { return sizeof(system_block); }
  size_t alignment() const noexcept override { return alignof(system_block); }

  schedule_op op() const noexcept override { return m_op; }

private:
  [[no_unique_address]] start_storage m_start;
  [[no_unique_address]] update_storage m_update;
  [[no_unique_address]] stop_storage m_stop;
  system_ready_lifetime_hook<system_type> m_ready_lifetime_hook;
  schedule_op m_op; // TODO store within update storage.
};

} // namespace strobe::ecs
