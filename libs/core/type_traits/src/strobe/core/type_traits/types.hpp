#pragma once
#include <cstddef>
#include <type_traits>
#include <utility>

namespace strobe {

template <typename... Ts> struct Types {};

namespace details {

// =============================================================================
// Basic type-list inspection
// =============================================================================

template <typename T> struct IsTypes : std::false_type {};

template <typename... Ts> struct IsTypes<Types<Ts...>> : std::true_type {};

template <typename T> struct TypesSize;

template <typename... Ts>
struct TypesSize<Types<Ts...>>
    : std::integral_constant<std::size_t, sizeof...(Ts)> {};

template <typename T, typename List> struct TypesContains;

template <typename T, typename... Ts>
struct TypesContains<T, Types<Ts...>>
    : std::bool_constant<(std::is_same_v<T, Ts> || ...)> {};

// =============================================================================
// Boolean uniqueness check
// =============================================================================

template <typename List> struct TypesUnique;

template <> struct TypesUnique<Types<>> : std::true_type {};

template <typename T, typename... Rest>
struct TypesUnique<Types<T, Rest...>>
    : std::bool_constant<!TypesContains<T, Types<Rest...>>::value &&
                         TypesUnique<Types<Rest...>>::value> {};

// =============================================================================
// Concatenation
// =============================================================================

template <typename... Lists> struct TypesConcat;

template <> struct TypesConcat<> {
  using type = Types<>;
};

template <typename... Ts> struct TypesConcat<Types<Ts...>> {
  using type = Types<Ts...>;
};

template <typename... A, typename... B, typename... Rest>
struct TypesConcat<Types<A...>, Types<B...>, Rest...> {
  using type = typename TypesConcat<Types<A..., B...>, Rest...>::type;
};

// =============================================================================
// Disjointness
// =============================================================================

template <typename A, typename B> struct TypesDisjoint;

template <typename... As, typename B>
struct TypesDisjoint<Types<As...>, B>
    : std::bool_constant<(!TypesContains<As, B>::value && ...)> {};

// =============================================================================
// CV/ref normalization
// =============================================================================

template <typename List> struct TypesRemoveCvref;

template <typename... Ts> struct TypesRemoveCvref<Types<Ts...>> {
  using type = Types<std::remove_cvref_t<Ts>...>;
};

// =============================================================================
// Filter
// =============================================================================

template <typename List, template <typename> typename Predicate>
struct TypesFilter;

template <template <typename> typename Predicate>
struct TypesFilter<Types<>, Predicate> {
  using type = Types<>;
};

template <typename T, typename... Rest, template <typename> typename Predicate>
struct TypesFilter<Types<T, Rest...>, Predicate> {
private:
  using tail = typename TypesFilter<Types<Rest...>, Predicate>::type;
  using head = std::conditional_t<Predicate<T>::value, Types<T>, Types<>>;

public:
  using type = typename TypesConcat<head, tail>::type;
};

// =============================================================================
// Transform
// =============================================================================

template <typename List, template <typename> typename Mapper>
struct TypesTransform;

template <typename... Ts, template <typename> typename Mapper>
struct TypesTransform<Types<Ts...>, Mapper> {
  using type = Types<typename Mapper<Ts>::type...>;
};

// =============================================================================
// Remove one type from a list
// =============================================================================
//
// Removes all exact occurrences of T from List.
//

template <typename T, typename List> struct TypesRemove;

template <typename T> struct TypesRemove<T, Types<>> {
  using type = Types<>;
};

template <typename T, typename U, typename... Rest>
struct TypesRemove<T, Types<U, Rest...>> {
private:
  using tail = typename TypesRemove<T, Types<Rest...>>::type;

public:
  using type = std::conditional_t<std::is_same_v<T, U>, tail,
                                  typename TypesConcat<Types<U>, tail>::type>;
};

// =============================================================================
// Make unique
// =============================================================================
//
// Preserves the first occurrence order.
//
// Example:
//   Types<A, B, A, C, B> -> Types<A, B, C>
//

template <typename List> struct TypesMakeUnique;

template <> struct TypesMakeUnique<Types<>> {
  using type = Types<>;
};

template <typename T, typename... Rest>
struct TypesMakeUnique<Types<T, Rest...>> {
private:
  using rest_without_t = typename TypesRemove<T, Types<Rest...>>::type;
  using unique_rest = typename TypesMakeUnique<rest_without_t>::type;

public:
  using type = typename TypesConcat<Types<T>, unique_rest>::type;
};

// =============================================================================
// Subtract
// =============================================================================
//
// A - B: keeps all types from A that are not contained in B.
//

template <typename A, typename B> struct TypesSubtract;

template <typename B> struct TypesSubtract<Types<>, B> {
  using type = Types<>;
};

template <typename T, typename... Rest, typename B>
struct TypesSubtract<Types<T, Rest...>, B> {
private:
  using tail = typename TypesSubtract<Types<Rest...>, B>::type;

public:
  using type = std::conditional_t<TypesContains<T, B>::value, tail,
                                  typename TypesConcat<Types<T>, tail>::type>;
};

// =============================================================================
// Union
// =============================================================================
//
// Concatenates all lists and removes duplicates while preserving first
// occurrence order.
//

template <typename... Lists> struct TypesUnion {
private:
  using concatenated = typename TypesConcat<Lists...>::type;

public:
  using type = typename TypesMakeUnique<concatenated>::type;
};

// =============================================================================
// Intersection
// =============================================================================
//
// Keeps all types from A that are contained in B.
// Duplicates from A are preserved. Use TypesMakeUnique separately if needed.
//

template <typename A, typename B> struct TypesIntersection;

template <typename B> struct TypesIntersection<Types<>, B> {
  using type = Types<>;
};

template <typename T, typename... Rest, typename B>
struct TypesIntersection<Types<T, Rest...>, B> {
private:
  using tail = typename TypesIntersection<Types<Rest...>, B>::type;

public:
  using type =
      std::conditional_t<TypesContains<T, B>::value,
                         typename TypesConcat<Types<T>, tail>::type, tail>;
};

// =============================================================================
// Subset
// =============================================================================

template <typename A, typename B> struct TypesSubset;

template <typename... As, typename B>
struct TypesSubset<Types<As...>, B>
    : std::bool_constant<(TypesContains<As, B>::value && ...)> {};

} // namespace details

// =============================================================================
// Public interface
// =============================================================================

template <typename T>
concept types = details::IsTypes<std::remove_cvref_t<T>>::value;

// -----------------------------------------------------------------------------
// Basic inspection
// -----------------------------------------------------------------------------

template <types List>
inline constexpr std::size_t types_size_v =
    details::TypesSize<std::remove_cvref_t<List>>::value;

template <types List>
inline constexpr bool types_empty_v = types_size_v<List> == 0;

template <typename T, types List>
inline constexpr bool types_contains_v =
    details::TypesContains<T, std::remove_cvref_t<List>>::value;

template <types List>
inline constexpr bool types_unique_v =
    details::TypesUnique<std::remove_cvref_t<List>>::value;

// -----------------------------------------------------------------------------
// List construction / transformation
// -----------------------------------------------------------------------------

template <types... Lists>
using types_concat_t =
    typename details::TypesConcat<std::remove_cvref_t<Lists>...>::type;

template <types A, types B>
inline constexpr bool types_disjoint_v =
    details::TypesDisjoint<std::remove_cvref_t<A>,
                           std::remove_cvref_t<B>>::value;

template <types A, types B>
inline constexpr bool types_overlap_v = !types_disjoint_v<A, B>;

template <types List>
using types_remove_cvref_t =
    typename details::TypesRemoveCvref<std::remove_cvref_t<List>>::type;

template <types List, template <typename> typename Predicate>
using types_filter_t =
    typename details::TypesFilter<std::remove_cvref_t<List>, Predicate>::type;

template <types List, template <typename> typename Mapper>
using types_transform_t =
    typename details::TypesTransform<std::remove_cvref_t<List>, Mapper>::type;

template <typename T, types List>
using types_remove_t =
    typename details::TypesRemove<T, std::remove_cvref_t<List>>::type;

template <types List>
using types_unique_t =
    typename details::TypesMakeUnique<std::remove_cvref_t<List>>::type;

template <types A, types B>
using types_subtract_t =
    typename details::TypesSubtract<std::remove_cvref_t<A>,
                                    std::remove_cvref_t<B>>::type;

template <types... Lists>
using types_union_t =
    typename details::TypesUnion<std::remove_cvref_t<Lists>...>::type;

template <types A, types B>
using types_intersection_t =
    typename details::TypesIntersection<std::remove_cvref_t<A>,
                                        std::remove_cvref_t<B>>::type;

// -----------------------------------------------------------------------------
// Relations
// -----------------------------------------------------------------------------

template <types A, types B>
inline constexpr bool types_subset_v =
    details::TypesSubset<std::remove_cvref_t<A>, std::remove_cvref_t<B>>::value;

// -----------------------------------------------------------------------------
// Conditional single-type list
// -----------------------------------------------------------------------------

template <bool Condition, typename T>
using types_if_t = std::conditional_t<Condition, Types<T>, Types<>>;

namespace details {

template <typename List> struct for_each_type_impl;

template <typename... Ts> struct for_each_type_impl<Types<Ts...>> {
  template <typename Fn> static void apply(Fn &&fn) noexcept {
    (std::forward<Fn>(fn).template operator()<Ts>(), ...);
  }
};
} // namespace details

template <typename List, typename Fn> void for_each_type(Fn &&fn) noexcept {
  details::for_each_type_impl<std::remove_cvref_t<List>>::apply(
      std::forward<Fn>(fn));
}

} // namespace strobe
