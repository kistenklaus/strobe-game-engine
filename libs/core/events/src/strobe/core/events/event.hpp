#pragma once

#include <concepts>
#include <utility>
namespace strobe::events {

template <typename E>
concept Event = requires(const E& e) {
  typename E::payload_type;
  { e.payload() } -> std::same_as<const typename E::payload_type&>;
  { e.canceled() } -> std::convertible_to<bool>;
} && std::constructible_from<E, typename E::payload_type>;

}  // namespace strobe::events
