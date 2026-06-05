#pragma once

#include "strobe/core/type_traits/member_function_traits.hpp"
#include "strobe/core/type_traits/types.hpp"
#include "strobe/ecs/object/object_traits.hpp"
#include "strobe/ecs/object/object_utils.hpp"
#include "strobe/ecs/resource.hpp"
#include "strobe/ecs/scheduler/op_scope.hpp"
#include "strobe/ecs/scheduler/scheduler.hpp"
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <type_traits>

namespace strobe::ecs {

struct Universe;

namespace details {

template <typename Args> struct object_typelist_valid;

template <typename... Args>
struct object_typelist_valid<Types<Args...>>
    : std::bool_constant<(object<Args> && ...)> {};

template <typename Args, bool Valid> struct object_typelist_traits_impl;

template <typename... Args>
struct object_typelist_traits_impl<Types<Args...>, false> {
  using types = Types<Args...>;
  using tuple_type = std::tuple<>;
  using memory_order_types = Types<>;
  static constexpr bool valid = false;
  static constexpr size_t acquire_count = 0;
  static constexpr size_t release_count = 0;
  static constexpr size_t acq_rel_count = 0;
};

template <typename... Args>
struct object_typelist_traits_impl<Types<Args...>, true> {
  using types = Types<Args...>;
  using tuple_type = std::tuple<object_arg_t<Args>...>;
  using memory_order_types = Types<object_memory_order_t<Args>...>;
  using memory_scope_type =
      operation_scope_desc<object_memory_order_t<Args>...>;

  static constexpr bool valid = true;
  static constexpr std::size_t acquire_count =
      (std::size_t{0} + ... + object_memory_order_t<Args>::acquire_count);
  static constexpr std::size_t release_count =
      (std::size_t{0} + ... + object_memory_order_t<Args>::release_count);
  static constexpr std::size_t acq_rel_count =
      (std::size_t{0} + ... + object_memory_order_t<Args>::acq_rel_count);
};

} // namespace details

template <typename Args>
struct object_typelist_traits
    : details::object_typelist_traits_impl<
          std::remove_cvref_t<Args>,
          details::object_typelist_valid<std::remove_cvref_t<Args>>::value> {};

namespace details {

template <typename Fn, bool Exists> struct object_function_traits_impl;

template <typename Fn> struct object_function_traits_impl<Fn, false> {
  static constexpr bool exists = false;
  using function = void;
  using return_type = void;
  using argument_types = Types<>;
  using memory_order_types = Types<>;
  static constexpr bool return_valid = true;
  static constexpr bool arguments_valid = true;
  static constexpr bool is_noexcept = true;
  static constexpr bool is_const = false;
  static constexpr bool is_volatile = false;
  static constexpr bool is_ref_qualified = false;
  static constexpr bool valid = true;
  static constexpr std::size_t acquire_count = 0;
  static constexpr std::size_t release_count = 0;
  static constexpr std::size_t acq_rel_count = 0;
};

template <typename Fn> struct object_function_traits_impl<Fn, true> {
  using function = std::remove_cvref_t<Fn>;
  using return_type = member_function_return_t<function>;
  using argument_types = member_function_argument_types_t<function>;
  using argument_traits = object_typelist_traits<argument_types>;
  static constexpr bool exists = true;
  using memory_order_types = typename argument_traits::memory_order_types;
  static constexpr bool return_valid =
      std::is_same_v<return_type, void> ||
      std::is_same_v<return_type, std::uint32_t>;
  static constexpr bool arguments_valid = argument_traits::valid;
  static constexpr bool is_noexcept =
      strobe::member_function_is_noexcept_v<function>;
  static constexpr bool is_const = strobe::member_function_is_const_v<function>;
  static constexpr bool is_volatile =
      strobe::member_function_is_volatile_v<function>;
  static constexpr bool is_ref_qualified =
      strobe::member_function_is_ref_qualified_v<function>;
  static constexpr bool valid = return_valid && arguments_valid &&
                                is_noexcept && !is_volatile &&
                                !is_ref_qualified;
  static constexpr std::size_t acquire_count = argument_traits::acquire_count;
  static constexpr std::size_t release_count = argument_traits::release_count;
  static constexpr std::size_t acq_rel_count = argument_traits::acq_rel_count;
};

} // namespace details

template <typename Fn>
struct object_function_traits
    : details::object_function_traits_impl<Fn, member_function_pointer<Fn>> {};

namespace details {

template <auto Fn, typename Args> struct object_function_impl;

template <auto Fn, typename... Args>
struct object_function_impl<Fn, Types<Args...>> {
  using member_function = std::remove_cvref_t<decltype(Fn)>;
  using traits = object_function_traits<member_function>;
  using class_type = member_function_class_t<member_function>;
  using return_type = typename traits::return_type;
  using argument_traits = typename traits::argument_traits;
  struct job_storage {
    using tuple_type = typename argument_traits::tuple_type;
    explicit job_storage(Universe *universe) noexcept
        : args(make_object<Args>(universe)...) {}
    tuple_type args;
  };

  struct job {
    job_storage *m_storage = nullptr;
    class_type *m_object = nullptr;
    std::uint32_t operator()(std::uint32_t) noexcept {
      return std::apply(
          [this](auto &...args) noexcept -> std::uint32_t {
            if constexpr (std::is_void_v<return_type>) {
              (m_object->*Fn)(args...);
              return 0;
            } else {
              return (m_object->*Fn)(args...);
            }
          },
          m_storage->args);
    }
  };

  static_assert(sizeof(job) <= 16);
  struct scope_fn {
    explicit scope_fn(Universe *universe) noexcept
        : m_scope(make_object_access<Args>(universe)...) {}

    void operator()(auto &scope) const noexcept { m_scope.apply(scope); }

    typename argument_traits::memory_scope_type m_scope;
  };

  explicit object_function_impl(class_type *object, Universe *universe) noexcept
      : m_storage(universe),
        m_job{
            .m_storage = std::addressof(m_storage),
            .m_object = object,
        },
        m_scope(universe) {}

  object_function_impl(const object_function_impl &) = delete;
  object_function_impl &operator=(const object_function_impl &) = delete;
  object_function_impl(object_function_impl &&) = delete;
  object_function_impl &operator=(object_function_impl &&) = delete;

  void submit(Scheduler *scheduler) noexcept {
    scheduler->submit(m_scope, m_job);
  }

private:
  job_storage m_storage;
  job m_job;
  scope_fn m_scope;
};

} // namespace details

template <auto Fn>
struct object_function
    : details::object_function_impl<
          Fn, typename object_function_traits<
                  std::remove_cvref_t<decltype(Fn)>>::argument_types> {
private:
  using base = details::object_function_impl<
      Fn, typename object_function_traits<
              std::remove_cvref_t<decltype(Fn)>>::argument_types>;

public:
  using base::base;
};

namespace details {

template <auto Fn, typename Args> struct sync_object_function_impl;

template <auto Fn, typename... Args>
struct sync_object_function_impl<Fn, Types<Args...>> {
  using member_function = std::remove_cvref_t<decltype(Fn)>;
  using traits = object_function_traits<member_function>;
  using class_type = member_function_class_t<member_function>;
  using return_type = typename traits::return_type;
  using argument_traits = typename traits::argument_traits;
  struct job_storage {
    using tuple_type = typename argument_traits::tuple_type;
    explicit job_storage(Universe *regs) noexcept
        : args(make_object<Args>(regs)...) {}
    tuple_type args;
  };

  struct job {
    job_storage *m_storage = nullptr;
    class_type *m_object = nullptr;
    std::uint32_t operator()(std::uint32_t) noexcept {
      return std::apply(
          [this](auto &...args) noexcept -> std::uint32_t {
            if constexpr (std::is_void_v<return_type>) {
              (m_object->*Fn)(args...);
              return 0;
            } else {
              return (m_object->*Fn)(args...);
            }
          },
          m_storage->args);
    }
  };

  static_assert(sizeof(job) <= 16);
  struct scope_fn {
    explicit scope_fn(location loc, Universe *regs) noexcept
        : m_loc(loc), m_scope(make_object_access<Args>(regs)...) {}

    void operator()(auto &scope) const noexcept {
      scope.acq_rel(m_loc);
      m_scope.apply(scope);
    }

    location m_loc;
    typename argument_traits::memory_scope_type m_scope;
  };

  explicit sync_object_function_impl(class_type *object, location loc,
                                     Universe *regs) noexcept
      : m_storage(regs),
        m_job{
            .m_storage = std::addressof(m_storage),
            .m_object = object,
        },
        m_scope(loc, regs) {}

  sync_object_function_impl(const sync_object_function_impl &) = delete;
  sync_object_function_impl &
  operator=(const sync_object_function_impl &) = delete;
  sync_object_function_impl(sync_object_function_impl &&) = delete;
  sync_object_function_impl &operator=(sync_object_function_impl &&) = delete;

  void submit(Scheduler *scheduler) noexcept {
    scheduler->submit(m_scope, m_job);
  }

private:
  job_storage m_storage;
  job m_job;
  scope_fn m_scope;
};

} // namespace details

template <auto Fn>
struct sync_object_function
    : details::sync_object_function_impl<
          Fn, typename object_function_traits<
                  std::remove_cvref_t<decltype(Fn)>>::argument_types> {
private:
  using base = details::sync_object_function_impl<
      Fn, typename object_function_traits<
              std::remove_cvref_t<decltype(Fn)>>::argument_types>;

public:
  using base::base;
};

template <typename S>
concept stateless_system_object =
    std::is_empty_v<std::remove_cvref_t<S>> &&
    std::is_trivially_default_constructible_v<std::remove_cvref_t<S>> &&
    std::is_trivially_destructible_v<std::remove_cvref_t<S>>;

namespace details {

template <auto Fn, typename Args> struct stateless_object_function_impl;

template <auto Fn, typename... Args>
struct stateless_object_function_impl<Fn, Types<Args...>> {
  using member_function = std::remove_cvref_t<decltype(Fn)>;
  using traits = object_function_traits<member_function>;
  using class_type = member_function_class_t<member_function>;
  using return_type = typename traits::return_type;
  using argument_traits = typename traits::argument_traits;

  static_assert(stateless_system_object<class_type>);
  static_assert(traits::valid);

  struct job_storage {
    using tuple_type = typename argument_traits::tuple_type;

    explicit job_storage(Universe *regs) noexcept
        : args(make_object<Args>(regs)...) {}

    tuple_type args;
  };

  struct job {
    job_storage *m_storage = nullptr;

    std::uint32_t operator()(std::uint32_t) noexcept {
      return std::apply(
          [](auto &...args) noexcept -> std::uint32_t {
            class_type object{};

            if constexpr (std::is_void_v<return_type>) {
              (object.*Fn)(args...);
              return 0;
            } else {
              return (object.*Fn)(args...);
            }
          },
          m_storage->args);
    }
  };

  static_assert(sizeof(job) <= 16);

  struct scope_fn {
    explicit scope_fn(Universe *regs) noexcept
        : m_scope(make_object_access<Args>(regs)...) {}

    void operator()(auto &scope) const noexcept { m_scope.apply(scope); }

    typename argument_traits::memory_scope_type m_scope;
  };

  explicit stateless_object_function_impl(Universe *regs) noexcept
      : m_storage(regs),
        m_job{
            .m_storage = std::addressof(m_storage),
        },
        m_scope(regs) {}

  stateless_object_function_impl(const stateless_object_function_impl &) =
      delete;

  stateless_object_function_impl &
  operator=(const stateless_object_function_impl &) = delete;

  stateless_object_function_impl(stateless_object_function_impl &&) = delete;

  stateless_object_function_impl &
  operator=(stateless_object_function_impl &&) = delete;

  void submit(Scheduler *scheduler) noexcept {
    scheduler->submit(m_scope, m_job);
  }

private:
  job_storage m_storage;
  job m_job;
  scope_fn m_scope;
};

} // namespace details

template <auto Fn>
struct stateless_object_function
    : details::stateless_object_function_impl<
          Fn, typename object_function_traits<
                  std::remove_cvref_t<decltype(Fn)>>::argument_types> {
private:
  using base = details::stateless_object_function_impl<
      Fn, typename object_function_traits<
              std::remove_cvref_t<decltype(Fn)>>::argument_types>;

public:
  using base::base;
};

namespace details {

struct empty_job_storage_ref {};

template <typename ArgumentTraits, bool Valid>
struct memory_scope_type_or_empty {
  using type = operation_scope_desc<>;
};

template <typename ArgumentTraits>
struct memory_scope_type_or_empty<ArgumentTraits, true> {
  using type = typename ArgumentTraits::memory_scope_type;
};

template <auto Fn, typename Args> struct sync_stateless_object_function_impl;

template <auto Fn, typename... Args>
struct sync_stateless_object_function_impl<Fn, Types<Args...>> {
  using member_function = std::remove_cvref_t<decltype(Fn)>;
  using traits = object_function_traits<member_function>;
  using class_type = member_function_class_t<member_function>;
  using return_type = typename traits::return_type;
  using argument_traits = typename traits::argument_traits;

  static_assert(
      stateless_system_object<class_type>,
      "sync_stateless_object_function<Fn>: Fn's class type must be empty, "
      "trivially default constructible, and trivially destructible");

  static_assert(
      traits::valid,
      "sync_stateless_object_function<Fn>: Fn must be a valid ECS object "
      "member function");

  static constexpr bool has_stored_args = sizeof...(Args) != 0;

  using memory_scope_type =
      typename memory_scope_type_or_empty<argument_traits,
                                          argument_traits::valid>::type;

  struct job_storage {
    using tuple_type = typename argument_traits::tuple_type;

    explicit job_storage(Universe *regs) noexcept
        : args(make_object<Args>(regs)...) {}

    tuple_type args;
  };

  struct job {
    using storage_ref_type = std::conditional_t<has_stored_args, job_storage *,
                                                empty_job_storage_ref>;

    [[no_unique_address]] storage_ref_type m_storage{};

    static constexpr storage_ref_type
    make_storage_ref(job_storage *storage) noexcept {
      if constexpr (has_stored_args) {
        return storage;
      } else {
        static_cast<void>(storage);
        return {};
      }
    }

    template <typename... CallArgs>
    static std::uint32_t invoke(CallArgs &...args) noexcept {
      class_type object{};

      if constexpr (std::is_void_v<return_type>) {
        (object.*Fn)(args...);
        return 0;
      } else {
        return (object.*Fn)(args...);
      }
    }

    std::uint32_t operator()(std::uint32_t) noexcept {
      if constexpr (has_stored_args) {
        return std::apply(
            [](auto &...args) noexcept -> std::uint32_t {
              return job::invoke(args...);
            },
            m_storage->args);
      } else {
        return invoke();
      }
    }
  };

  static_assert(sizeof(job) <= 16);

  struct scope_fn {
    explicit scope_fn(location loc, Universe *regs) noexcept
        : m_loc(loc), m_scope(make_object_access<Args>(regs)...) {}

    void operator()(auto &scope) const noexcept {
      scope.acq_rel(m_loc);
      m_scope.apply(scope);
    }

    location m_loc;
    [[no_unique_address]] memory_scope_type m_scope;
  };

  explicit sync_stateless_object_function_impl(location loc,
                                               Universe *regs) noexcept
      : m_storage(regs),
        m_job{
            .m_storage = job::make_storage_ref(std::addressof(m_storage)),
        },
        m_scope(loc, regs) {}

  sync_stateless_object_function_impl(
      const sync_stateless_object_function_impl &) = delete;

  sync_stateless_object_function_impl &
  operator=(const sync_stateless_object_function_impl &) = delete;

  sync_stateless_object_function_impl(sync_stateless_object_function_impl &&) =
      delete;

  sync_stateless_object_function_impl &
  operator=(sync_stateless_object_function_impl &&) = delete;

  void submit(Scheduler *scheduler) noexcept {
    scheduler->submit(m_scope, m_job);
  }

private:
  [[no_unique_address]] job_storage m_storage;
  [[no_unique_address]] job m_job;
  scope_fn m_scope;
};

} // namespace details

template <auto Fn>
struct sync_stateless_object_function
    : details::sync_stateless_object_function_impl<
          Fn, typename object_function_traits<
                  std::remove_cvref_t<decltype(Fn)>>::argument_types> {
private:
  using base = details::sync_stateless_object_function_impl<
      Fn, typename object_function_traits<
              std::remove_cvref_t<decltype(Fn)>>::argument_types>;

public:
  using base::base;
};

namespace details {

template <auto Fn, typename Args> struct stateful_object_function_impl;

template <auto Fn, typename... Args>
struct stateful_object_function_impl<Fn, Types<Args...>> {
  using member_function = std::remove_cvref_t<decltype(Fn)>;

  using traits = object_function_traits<member_function>;

  using state_type = member_function_class_t<member_function>;

  static constexpr bool is_const_function =
      strobe::member_function_is_const_v<member_function>;

  using state_resource_type =
      std::conditional_t<is_const_function, Resource<const state_type>,
                         Resource<state_type>>;

  using return_type = typename traits::return_type;

  using argument_types = Types<state_resource_type, Args...>;

  using argument_traits = object_typelist_traits<argument_types>;

  static_assert(traits::valid, "stateful_object_function<Fn>: Fn must be a "
                               "valid ECS object member function");

  static_assert(argument_traits::valid,
                "stateful_object_function<Fn>: synthesized Resource<S>, "
                "Args... must be valid ECS objects");

  struct job_storage {
    using tuple_type = typename argument_traits::tuple_type;

    explicit job_storage(Universe *universe) noexcept
        : args(make_object<state_resource_type>(universe),
               make_object<Args>(universe)...) {}

    tuple_type args;
  };

  struct job {
    job_storage *m_storage = nullptr;

    template <typename StateArg, typename... CallArgs>
    static std::uint32_t invoke(StateArg &state, CallArgs &...args) noexcept {
      auto *object = state.operator->();

      if constexpr (std::is_void_v<return_type>) {
        (object->*Fn)(args...);
        return 0;
      } else {
        return (object->*Fn)(args...);
      }
    }

    std::uint32_t operator()(std::uint32_t) noexcept {
      return std::apply(
          [](auto &state, auto &...args) noexcept -> std::uint32_t {
            if constexpr (std::is_void_v<return_type>) {
              // Use whatever accessor Resource<T> provides here.
              ((*state).*Fn)(args...);

              return 0;
            } else {
              return ((*state).*Fn)(args...);
            }
          },
          m_storage->args);
    }
  };

  static_assert(sizeof(job) <= 16);

  struct scope_fn {
    explicit scope_fn(Universe *universe) noexcept
        : m_scope(make_object_access<state_resource_type>(universe),
                  make_object_access<Args>(universe)...) {}

    void operator()(auto &scope) const noexcept { m_scope.apply(scope); }

    typename argument_traits::memory_scope_type m_scope;
  };

  explicit stateful_object_function_impl(Universe *universe) noexcept
      : m_storage(universe),
        m_job{
            .m_storage = std::addressof(m_storage),
        },
        m_scope(universe) {}

  void submit(Scheduler *scheduler) noexcept {
    scheduler->submit(m_scope, m_job);
  }

private:
  job_storage m_storage;
  job m_job;
  scope_fn m_scope;
};

} // namespace details

template <auto Fn>
struct stateful_object_function
    : details::stateful_object_function_impl<
          Fn, typename object_function_traits<
                  std::remove_cvref_t<decltype(Fn)>>::argument_types> {
private:
  using base = details::stateful_object_function_impl<
      Fn, typename object_function_traits<
              std::remove_cvref_t<decltype(Fn)>>::argument_types>;

public:
  using base::base;
};

namespace details {

template <auto Fn, typename Args> struct sync_stateful_object_function_impl;

template <auto Fn, typename... Args>
struct sync_stateful_object_function_impl<Fn, Types<Args...>> {
  using member_function = std::remove_cvref_t<decltype(Fn)>;

  using traits = object_function_traits<member_function>;

  using state_type = member_function_class_t<member_function>;

  static constexpr bool is_const_function =
      strobe::member_function_is_const_v<member_function>;

  using state_resource_type =
      std::conditional_t<is_const_function, Resource<const state_type>,
                         Resource<state_type>>;

  using return_type = typename traits::return_type;

  using argument_types = Types<state_resource_type, Args...>;

  using argument_traits = object_typelist_traits<argument_types>;

  static_assert(
      traits::valid,
      "sync_stateful_object_function<Fn>: Fn must be a valid ECS object "
      "member function");

  static_assert(
      argument_traits::valid,
      "sync_stateful_object_function<Fn>: synthesized Resource<S>, Args... "
      "must be valid ECS objects");

  using memory_scope_type =
      typename memory_scope_type_or_empty<argument_traits,
                                          argument_traits::valid>::type;

  struct job_storage {
    using tuple_type = typename argument_traits::tuple_type;

    explicit job_storage(Universe *regs) noexcept
        : args(make_object<state_resource_type>(regs),
               make_object<Args>(regs)...) {}

    tuple_type args;
  };

  struct job {
    job_storage *m_storage = nullptr;

    template <typename StateArg, typename... CallArgs>
    static std::uint32_t invoke(StateArg &state, CallArgs &...args) noexcept {
      auto *object = state.operator->();

      if constexpr (std::is_void_v<return_type>) {
        (object->*Fn)(args...);

        return 0;
      } else {
        return (object->*Fn)(args...);
      }
    }

    std::uint32_t operator()(std::uint32_t) noexcept {
      return std::apply(
          [](auto &state, auto &...args) noexcept -> std::uint32_t {
            return job::invoke(state, args...);
          },
          m_storage->args);
    }
  };

  static_assert(sizeof(job) <= 16);

  struct scope_fn {
    explicit scope_fn(location loc, Universe *regs) noexcept
        : m_loc(loc), m_scope(make_object_access<state_resource_type>(regs),
                              make_object_access<Args>(regs)...) {}

    void operator()(auto &scope) const noexcept {
      scope.acq_rel(m_loc);
      m_scope.apply(scope);
    }

    location m_loc;

    [[no_unique_address]]
    memory_scope_type m_scope;
  };

  explicit sync_stateful_object_function_impl(location loc,
                                              Universe *regs) noexcept
      : m_storage(regs),
        m_job{
            .m_storage = std::addressof(m_storage),
        },
        m_scope(loc, regs) {}

  sync_stateful_object_function_impl(
      const sync_stateful_object_function_impl &) = delete;

  sync_stateful_object_function_impl &
  operator=(const sync_stateful_object_function_impl &) = delete;

  sync_stateful_object_function_impl(sync_stateful_object_function_impl &&) =
      delete;

  sync_stateful_object_function_impl &
  operator=(sync_stateful_object_function_impl &&) = delete;

  void submit(Scheduler *scheduler) noexcept {
    scheduler->submit(m_scope, m_job);
  }

private:
  [[no_unique_address]]
  job_storage m_storage;

  [[no_unique_address]]
  job m_job;

  scope_fn m_scope;
};

} // namespace details

template <auto Fn>
struct sync_stateful_object_function
    : details::sync_stateful_object_function_impl<
          Fn, typename object_function_traits<
                  std::remove_cvref_t<decltype(Fn)>>::argument_types> {
private:
  using base = details::sync_stateful_object_function_impl<
      Fn, typename object_function_traits<
              std::remove_cvref_t<decltype(Fn)>>::argument_types>;

public:
  using base::base;
};

} // namespace strobe::ecs
