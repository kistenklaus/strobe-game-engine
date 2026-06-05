#pragma once

#include "strobe/core/type_traits/dependent_false.hpp"
#include "strobe/ecs/object/object_traits.hpp"
#include "strobe/ecs/resource.hpp"
#include "strobe/ecs/resource/resource_registry.hpp"
#include "strobe/ecs/resource_commands.hpp"
#include "strobe/ecs/scheduler/location.hpp"
#include "strobe/ecs/universe.hpp"
#include <concepts>

namespace strobe::ecs {

template <object O> static O make_object(Universe *universe) {
  using object_type = object_arg_t<O>;
  using object_tag = typename object_type::object_tag;
  if constexpr (std::same_as<object_tag, resource_tag>) {
    using resource_type = typename object_type::resource_type;
    const resource_id id = universe->rreg.get_resource_id<resource_type>();
    void **ptr = universe->rreg.get_resource_ptr(id);
    return object_type{reinterpret_cast<resource_type **>(ptr)};
  } else if constexpr (std::same_as<object_tag,
                                    strobe::ecs::resource_cmd_tag>) {
    return object_type{universe};
  } else {
    static_assert(dependent_false_pack_v<object_type, object_tag>,
                  "make_object<O>: unsupported object tag");
  }
}

template <object O> static location get_object_location(Universe *universe) {
  using object_type = object_arg_t<O>;
  using object_tag = typename object_type::object_tag;
  if constexpr (std::same_as<object_tag, resource_tag>) {
    using resource_type = typename object_type::resource_type;
    const resource_id id = universe->rreg.get_resource_id<resource_type>();
    return universe->rreg.get_resource_location(id);
  } else if constexpr (std::same_as<object_tag, resource_cmd_tag>) {
    return universe->sr_location;
  } else {
    static_assert(dependent_false_pack_v<object_type, object_tag>,
                  "get_object_location<O>: unsupported object tag");
  }
}

template <object O>
static auto make_object_access(Universe *universe) noexcept {
  using object_type = object_arg_t<O>;
  using memory_order = typename object_type::memory_order;
  return memory_order{get_object_location<object_type>(universe)};
}

template <object O> static lifetime_id get_object_lifetime(Universe *universe) {
  using object_type = object_arg_t<O>;
  using object_tag = typename object_type::object_tag;
  if constexpr (std::same_as<object_tag, resource_tag>) {
    using resource_type = typename object_type::resource_type;
    const resource_id id = universe->rreg.get_resource_id<resource_type>();
    return universe->rreg.get_resource_lifetime(id);
  } else {
    static_assert(dependent_false_pack_v<object_type, object_tag>,
                  "get_object_lifetime<O>: unsupported object tag");
  }
}

} // namespace strobe::ecs
