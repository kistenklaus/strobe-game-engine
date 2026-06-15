#pragma once
#include "strobe/core/type_traits/dependent_false.hpp"
#include "strobe/ecs/object/object_traits.hpp"
#include "strobe/ecs/scheduler/op_scope.hpp"
#include "strobe/ecs/system/stateful_system.hpp"
#include "strobe/ecs/system/system_setup_traits.hpp"
#include "strobe/ecs/system/system_start_traits.hpp"
#include "strobe/ecs/system/system_stop_traits.hpp"
#include "strobe/ecs/system/system_teardown_traits.hpp"
#include "strobe/ecs/system/system_update_traits.hpp"
#include "strobe/ecs/universe.hpp"
#include <type_traits>

namespace strobe {

namespace ecs {

struct resource_cmd_tag {
  static constexpr bool is_object = true;
};

namespace details {

template <typename T, typename = void>
struct is_resource_commands_object : std::false_type {};

template <typename T>
struct is_resource_commands_object<
    T, std::void_t<typename object_arg_t<T>::object_tag>>
    : std::bool_constant<std::same_as<typename object_arg_t<T>::object_tag,
                                      resource_cmd_tag>> {};
} // namespace details

} // namespace ecs

class ResourceCommands {
public:
  using object_tag = ecs::resource_cmd_tag;
  using memory_order = ecs::acq_rel_access;

  explicit ResourceCommands(ecs::Universe *universe) : m_universe(universe) {}

  template <typename R, typename... Args> void create(Args &&...args) {
    using base_type = std::remove_cvref_t<R>;
    constexpr bool is_system = ecs::system_setup_exists_v<base_type> ||
                               ecs::system_start_exists_v<base_type> ||
                               ecs::system_update_exists_v<base_type> ||
                               ecs::system_stop_exists_v<base_type> ||
                               ecs::system_teardown_exists_v<base_type>;
    constexpr bool is_pure_system =
        is_system && std::is_empty_v<base_type> &&
        std::is_trivially_default_constructible_v<base_type> &&
        std::is_trivially_destructible_v<base_type>;
    if constexpr (is_pure_system) {
      static_assert(sizeof...(Args) == 0,
                    "ResourceCommands::create<R>: pure systems must not be "
                    "created with constructor arguments");
      m_universe->sreg.cmd_create<base_type>();
    } else if constexpr (is_system) {
      m_universe->rreg.cmd_emplace<base_type>(std::forward<Args>(args)...);
      m_universe->sreg.cmd_create<ecs::stateful_system<base_type>>();
    } else {
      m_universe->rreg.cmd_emplace<base_type>(std::forward<Args>(args)...);
    }
  }

  template <typename R> void destroy() {
    using base_type = std::remove_cvref_t<R>;
    constexpr bool is_system = ecs::system_setup_exists_v<base_type> ||
                               ecs::system_start_exists_v<base_type> ||
                               ecs::system_update_exists_v<base_type> ||
                               ecs::system_stop_exists_v<base_type> ||
                               ecs::system_teardown_exists_v<base_type>;
    constexpr bool is_pure_system =
        is_system && std::is_empty_v<base_type> &&
        std::is_trivially_default_constructible_v<base_type> &&
        std::is_trivially_destructible_v<base_type>;
    if constexpr (is_pure_system) {
      m_universe->sreg.cmd_destroy<base_type>();
    } else if constexpr (is_system) {
      m_universe->sreg.cmd_destroy<ecs::stateful_system<base_type>>();
      m_universe->rreg.cmd_destroy<base_type>();
    } else {
      m_universe->rreg.cmd_destroy<base_type>();
    }
  }

  template <typename S> void enable() {
    using base_type = std::remove_cvref_t<S>;
    constexpr bool is_system = ecs::system_setup_exists_v<base_type> ||
                               ecs::system_start_exists_v<base_type> ||
                               ecs::system_update_exists_v<base_type> ||
                               ecs::system_stop_exists_v<base_type> ||
                               ecs::system_teardown_exists_v<base_type>;
    constexpr bool is_pure_system =
        is_system && std::is_empty_v<base_type> &&
        std::is_trivially_default_constructible_v<base_type> &&
        std::is_trivially_destructible_v<base_type>;
    if constexpr (is_pure_system) {
      m_universe->sreg.cmd_enable<base_type>();
    } else if constexpr (is_system) {
      m_universe->sreg.cmd_enable<ecs::stateful_system<base_type>>();
    } else {
      static_assert(dependent_false_v<base_type>,
                    "ResourceCommands:::enable<S> is only valid for systems");
    }
  }

  template <typename S> void disable() {
    using base_type = std::remove_cvref_t<S>;
    constexpr bool is_system = ecs::system_setup_exists_v<base_type> ||
                               ecs::system_start_exists_v<base_type> ||
                               ecs::system_update_exists_v<base_type> ||
                               ecs::system_stop_exists_v<base_type> ||
                               ecs::system_teardown_exists_v<base_type>;
    constexpr bool is_pure_system =
        is_system && std::is_empty_v<base_type> &&
        std::is_trivially_default_constructible_v<base_type> &&
        std::is_trivially_destructible_v<base_type>;
    if constexpr (is_pure_system) {
      m_universe->sreg.cmd_disable<base_type>();
    } else if constexpr (is_system) {
      m_universe->sreg.cmd_disable<ecs::stateful_system<base_type>>();
    } else {
      static_assert(dependent_false_v<base_type>,
                    "ResourceCommands::disable<S> is only valid for systems");
    }
  }

private:
  ecs::Universe *m_universe;
};
static_assert(ecs::object<ResourceCommands>);

} // namespace strobe
