#pragma once

#include <type_traits>

#include "strobe/ecs/object/object_traits.hpp"
#include "strobe/ecs/scheduler/op_scope.hpp"

namespace strobe {

namespace ecs {

struct resource_tag {
  static constexpr bool is_object = true;
};

namespace details {

template <typename T, typename = void>
struct is_resource_object : std::false_type {};

template <typename T>
struct is_resource_object<T, std::void_t<typename object_arg_t<T>::object_tag>>
    : std::bool_constant<
          std::same_as<typename object_arg_t<T>::object_tag, resource_tag>> {};

} // namespace details

} // namespace ecs

template <typename R>
  requires(!std::is_reference_v<R> && !std::is_volatile_v<R>)
struct Resource {
public:
  using resource_type = std::remove_cvref_t<R>;

  using object_tag = ecs::resource_tag;
  using memory_order =
      std::conditional_t<std::is_const_v<R>, ecs::acquire_access,
                         ecs::acq_rel_access>;

  explicit Resource(resource_type **ptr) : m_ptr(ptr) {}

  R &get() const { return **m_ptr; }

  R *operator->() const { return *m_ptr; }

  R &operator*() const { return **m_ptr; }

private:
  resource_type **m_ptr;
};
static_assert(ecs::object<Resource<int>>);

} // namespace strobe
