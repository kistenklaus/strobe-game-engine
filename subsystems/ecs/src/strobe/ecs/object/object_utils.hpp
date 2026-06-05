#pragma once

#include "strobe/ecs/lifetime/lifetime_registry.hpp"
#include "strobe/ecs/object/object_traits.hpp"
#include "strobe/ecs/scheduler/location.hpp"

namespace strobe::ecs {

struct Universe;

template <object O> static O make_object(Universe *universe);

template <object O> static location get_object_location(Universe *universe);

template <object O> static auto make_object_access(Universe *universe) noexcept;

template <object O> static lifetime_id get_object_lifetime(Universe *universe);

} // namespace strobe::ecs
