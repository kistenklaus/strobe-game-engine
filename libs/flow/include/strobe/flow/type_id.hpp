#pragma once

#include <cstdint>
#include <string_view>

namespace strobe::flow {

/**
 * \brief Identifies an operation argument type.
 * \code{.cpp}
 * struct TypeId {
 *   std::uint64_t value;
 * };
 * \endcode
 */
struct TypeId {
  std::uint64_t value = 0;

  /**
   * \brief Compares type identifiers.
   * \code{.cpp}
   * friend constexpr bool operator==(TypeId, TypeId);
   * \endcode
   *
   * \return Whether both identifiers contain the same value.
   */
  friend constexpr bool operator==(TypeId, TypeId) = default;
};

namespace detail {
consteval std::uint64_t hash_type_name(std::string_view name) {
  std::uint64_t result = 14695981039346656037ull;
  for (const auto character : name) {
    result ^= static_cast<unsigned char>(character);
    result *= 1099511628211ull;
  }
  return result;
}
} // namespace detail

/**
 * \brief Returns the identifier used for a C++ type in the current build.
 * \code{.cpp}
 * template<typename T>
 * consteval TypeId type_id() noexcept;
 * \endcode
 *
 * \return Identifier derived from the compiler's canonical type signature.
 */
template <typename T> consteval TypeId type_id() noexcept {
#if defined(__clang__) || defined(__GNUC__)
  return TypeId{detail::hash_type_name(__PRETTY_FUNCTION__)};
#elif defined(_MSC_VER)
  return TypeId{detail::hash_type_name(__FUNCSIG__)};
#else
#error "flow::type_id requires a compiler type signature intrinsic"
#endif
}

} // namespace strobe::flow
