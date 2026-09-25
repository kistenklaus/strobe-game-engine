#pragma once

#include <concepts>

namespace strobe::events {

/**
 * \ingroup core
 * \brief Concept implemented by types accepted by EventDispatcher.
 * \code{.cpp}
 * template<typename E>
 * concept Event;
 * \endcode
 *
 * An event exposes a payload, reports whether dispatch has been canceled, and
 * is constructible from its payload type.
 */
template <typename E>
concept Event = requires(const E &e) {
  typename E::payload_type;
  { e.payload() } -> std::same_as<const typename E::payload_type &>;
  { e.canceled() } -> std::convertible_to<bool>;
} && std::constructible_from<E, typename E::payload_type>;

} // namespace strobe::events
