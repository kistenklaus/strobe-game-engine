#pragma once

#include <concepts>
#include <cstddef>
#include <fmt/base.h>
#include <type_traits>

namespace strobe::ecs {

template <typename T> using object_arg_t = std::remove_cvref_t<T>;
template <typename T>
using object_memory_order_t = typename object_arg_t<T>::memory_order;

template <typename T>
concept memory_order = requires {
  { std::remove_cvref_t<T>::acquire_count } -> std::convertible_to<std::size_t>;
  { std::remove_cvref_t<T>::release_count } -> std::convertible_to<std::size_t>;
  { std::remove_cvref_t<T>::acq_rel_count } -> std::convertible_to<std::size_t>;
};

template <typename T>
concept object =
    requires {
      typename object_arg_t<T>::object_tag;
      typename object_arg_t<T>::memory_order;
    } && object_arg_t<T>::object_tag::is_object &&
    memory_order<object_memory_order_t<T>>;




} // namespace strobe::ecs
