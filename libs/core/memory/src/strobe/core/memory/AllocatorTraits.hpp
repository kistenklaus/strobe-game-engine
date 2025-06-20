#pragma once

#include <cassert>
#include <concepts>
#include <cstddef>
#include <utility>

namespace strobe {
    template<typename A>
    concept Allocator =
            requires(A &a, std::size_t size, std::size_t align, void *ptr)
            {
                { a.allocate(size, align) } -> std::same_as<void *>;
                { a.deallocate(ptr, size, align) } -> std::same_as<void>;
            };

    template<typename A>
    concept ReAllocator =
            Allocator<A> && requires(A a, void *ptr, std::size_t oldSize,
                                     std::size_t newSize, std::size_t align)
            {
                { a.reallocate(ptr, oldSize, newSize, align) } -> std::same_as<void *>;
            };

    template<typename A>
    concept OverAllocator =
            Allocator<A> && requires(A a, std::size_t size, std::size_t align)
            {
                {
                    a.allocate_at_least(size, align)
                } -> std::same_as<std::pair<void *, std::size_t> >;
            };

    template<typename A>
    concept OwningAllocator = Allocator<A> && requires(const A a, void *ptr)
    {
        { a.owns(ptr) } -> std::same_as<bool>;
    };

    template<typename A>
    concept SizeIndepdententAllocator = Allocator<A> && requires(A a, void *ptr)
    {
        { a.deallocate(ptr) };
    };

    template<typename A>
    concept ComparibleAllocator = Allocator<A> && std::equality_comparable<A>;

    template<Allocator A>
    struct AllocatorTraits {
        template<typename T>
        using pointer = T *;
        template<typename T>
        using const_pointer = const T *;

        static inline void *allocate(A &a, std::size_t size, std::size_t align) {
            return a.allocate(size, align);
        }

        static inline void deallocate(A &a, void *ptr, std::size_t size,
                                      std::size_t align) {
            a.deallocate(ptr, size, align);
        }

        static inline void size_independent_deallocate(A &a, void *ptr)
            requires SizeIndepdententAllocator<A> {
            return a.deallocate(ptr);
        }

        template<typename T>
        static inline T *allocate(A &a, std::size_t n = 1) {
            return static_cast<T *>(allocate(a, n * sizeof(T), alignof(T)));
        }

        template<typename T>
        static inline void deallocate(A &a, T *ptr, std::size_t n = 1) {
            deallocate(a, ptr, n * sizeof(T), alignof(T));
        }

        static bool owns(A &a, void *ptr)
            requires OwningAllocator<A> {
            return a.owns(ptr);
        }

        // Copy semantics
        static inline A select_on_container_copy_construction(const A &a) {
            if (requires { a.select_on_container_copy_construction(); }) {
                return a.select_on_container_copy_construction();
            } else {
                return a;
            }
        }

    private:
        // --- propagate_on_container_copy_assignment (POCCA) ---
        template <typename U, typename = void>
        struct has_pocca : std::false_type {};

        template <typename U>
        struct has_pocca<U, std::void_t<decltype(U::propagate_on_container_copy_assignment)>>
            : std::is_convertible<decltype(U::propagate_on_container_copy_assignment), bool> {};

        template <typename U>
        static constexpr bool pocca_value() noexcept {
            if constexpr (has_pocca<U>::value) {
                return static_cast<bool>(U::propagate_on_container_copy_assignment);
            } else {
                return true;
            }
        }

        // --- propagate_on_container_move_assignment (POMCA) ---
        template <typename U, typename = void>
        struct has_pomca : std::false_type {};

        template <typename U>
        struct has_pomca<U, std::void_t<decltype(U::propagate_on_container_move_assignment)>>
            : std::is_convertible<decltype(U::propagate_on_container_move_assignment), bool> {};

        template <typename U>
        static constexpr bool pomca_value() noexcept {
            if constexpr (has_pomca<U>::value) {
                return static_cast<bool>(U::propagate_on_container_move_assignment);
            } else {
                return true;
            }
        }

        // --- is_always_equal ---
        template <typename U, typename = void>
        struct has_is_always_equal : std::false_type {};

        template <typename U>
        struct has_is_always_equal<U, std::void_t<decltype(U::is_always_equal)>>
            : std::is_convertible<decltype(U::is_always_equal), bool> {};

        template <typename U>
        static constexpr bool is_always_equal_value() noexcept {
            if constexpr (has_is_always_equal<U>::value) {
                return static_cast<bool>(U::is_always_equal);
            } else {
                return false;
            }
        }


    public:
        static constexpr bool propagate_on_container_copy_assignment =
                pocca_value<A>();

        static constexpr bool propagate_on_container_move_assignment = pomca_value<A>();

        static constexpr bool is_always_equal =
                !ComparibleAllocator<A> && is_always_equal_value<A>();
    };

    template<class L, class R>
    constexpr bool alloc_equals(const L &lhs, const R &rhs) noexcept {
        /* ---------- same type ------------------------------------------------- */
        if constexpr (std::same_as<L, R>) {
            if constexpr (AllocatorTraits<L>::is_always_equal) {
                return true;
            } else if constexpr (ComparibleAllocator<L>) {
                return lhs == rhs; // (1)
            };
        }
        return false;
    }
} // namespace strobe
