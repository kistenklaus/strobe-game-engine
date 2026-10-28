#pragma once

#include "strobe/flow/flow.hpp"
#include "strobe/flow/resource.hpp"
#include <concepts>
#include <cstdint>
#include <memory>
#include <utility>

namespace strobe::flow {

namespace detail {

template <std::integral int_type> class integer_resource {
public:
  integer_resource(int_type v) noexcept
      : m_runtime(flow::resource<int_type>()) {
    flow::construct(m_runtime, [v](int_type *ptr) {
      ZoneScopedN("integer_resource::integer_resource");
      std::construct_at(ptr, v);
    });
  }
  integer_resource(flow::Resource<int_type> res) noexcept : m_runtime(res) {}

  flow::Resource<int_type> runtime() const noexcept { return m_runtime; }

  friend integer_resource operator+(const integer_resource &lhs,
                                    const integer_resource &rhs) noexcept {
    return binary(lhs, rhs, [](int_type lhs, int_type rhs) noexcept {
      return static_cast<int_type>(lhs + rhs);
    });
  }

  friend integer_resource operator-(const integer_resource &lhs,
                                    const integer_resource &rhs) noexcept {
    return binary(lhs, rhs, [](int_type lhs, int_type rhs) noexcept {
      return static_cast<int_type>(lhs - rhs);
    });
  }

  friend integer_resource operator*(const integer_resource &lhs,
                                    const integer_resource &rhs) noexcept {
    return binary(lhs, rhs, [](int_type lhs, int_type rhs) noexcept {
      return static_cast<int_type>(lhs * rhs);
    });
  }

  friend integer_resource operator/(const integer_resource &lhs,
                                    const integer_resource &rhs) noexcept {
    return binary(lhs, rhs, [](int_type lhs, int_type rhs) noexcept {
      return static_cast<int_type>(lhs / rhs);
    });
  }

  friend integer_resource operator%(const integer_resource &lhs,
                                    const integer_resource &rhs) noexcept {
    return binary(lhs, rhs, [](int_type lhs, int_type rhs) noexcept {
      return static_cast<int_type>(lhs % rhs);
    });
  }

  friend integer_resource operator&(const integer_resource &lhs,
                                    const integer_resource &rhs) noexcept {
    return binary(lhs, rhs, [](int_type lhs, int_type rhs) noexcept {
      return static_cast<int_type>(lhs & rhs);
    });
  }

  friend integer_resource operator|(const integer_resource &lhs,
                                    const integer_resource &rhs) noexcept {
    return binary(lhs, rhs, [](int_type lhs, int_type rhs) noexcept {
      return static_cast<int_type>(lhs | rhs);
    });
  }

  friend integer_resource operator^(const integer_resource &lhs,
                                    const integer_resource &rhs) noexcept {
    return binary(lhs, rhs, [](int_type lhs, int_type rhs) noexcept {
      return static_cast<int_type>(lhs ^ rhs);
    });
  }

  friend integer_resource operator<<(const integer_resource &lhs,
                                     const integer_resource &rhs) noexcept {
    return binary(lhs, rhs, [](int_type lhs, int_type rhs) noexcept {
      return static_cast<int_type>(lhs << rhs);
    });
  }

  friend integer_resource operator>>(const integer_resource &lhs,
                                     const integer_resource &rhs) noexcept {
    return binary(lhs, rhs, [](int_type lhs, int_type rhs) noexcept {
      return static_cast<int_type>(lhs >> rhs);
    });
  }

  friend integer_resource operator+(const integer_resource &value) noexcept {
    return value;
  }

  friend integer_resource operator-(const integer_resource &value) noexcept
    requires std::signed_integral<int_type>
  {
    return unary(value, [](int_type input) noexcept {
      return static_cast<int_type>(-input);
    });
  }

  friend integer_resource operator~(const integer_resource &value) noexcept {
    return unary(value, [](int_type input) noexcept {
      return static_cast<int_type>(~input);
    });
  }

  friend flow::Resource<bool> operator==(const integer_resource &lhs,
                                         const integer_resource &rhs) noexcept {
    return compare(lhs, rhs, [](int_type lhs, int_type rhs) noexcept {
      return lhs == rhs;
    });
  }

  friend flow::Resource<bool> operator!=(const integer_resource &lhs,
                                         const integer_resource &rhs) noexcept {
    return compare(lhs, rhs, [](int_type lhs, int_type rhs) noexcept {
      return lhs != rhs;
    });
  }

  friend flow::Resource<bool> operator<(const integer_resource &lhs,
                                        const integer_resource &rhs) noexcept {
    return compare(lhs, rhs, [](int_type lhs, int_type rhs) noexcept {
      return lhs < rhs;
    });
  }

  friend flow::Resource<bool> operator<=(const integer_resource &lhs,
                                         const integer_resource &rhs) noexcept {
    return compare(lhs, rhs, [](int_type lhs, int_type rhs) noexcept {
      return lhs <= rhs;
    });
  }

  friend flow::Resource<bool> operator>(const integer_resource &lhs,
                                        const integer_resource &rhs) noexcept {
    return compare(lhs, rhs, [](int_type lhs, int_type rhs) noexcept {
      return lhs > rhs;
    });
  }

  friend flow::Resource<bool> operator>=(const integer_resource &lhs,
                                         const integer_resource &rhs) noexcept {
    return compare(lhs, rhs, [](int_type lhs, int_type rhs) noexcept {
      return lhs >= rhs;
    });
  }

  friend flow::Resource<bool>
  operator!(const integer_resource &value) noexcept {
    return predicate(value, [](int_type input) noexcept { return !input; });
  }

  friend flow::Resource<bool> operator&&(const integer_resource &lhs,
                                         const integer_resource &rhs) noexcept {
    return compare(lhs, rhs, [](int_type lhs, int_type rhs) noexcept {
      return static_cast<bool>(lhs) && static_cast<bool>(rhs);
    });
  }

  friend flow::Resource<bool> operator||(const integer_resource &lhs,
                                         const integer_resource &rhs) noexcept {
    return compare(lhs, rhs, [](int_type lhs, int_type rhs) noexcept {
      return static_cast<bool>(lhs) || static_cast<bool>(rhs);
    });
  }

private:
  template <typename Operation>
  static integer_resource binary(const integer_resource &lhs,
                                 const integer_resource &rhs,
                                 Operation operation) noexcept {
    auto result = flow::resource<int_type>();
    flow::construct(result, flow::read(lhs.m_runtime),
                    flow::read(rhs.m_runtime),
                    [operation = std::move(operation)](

                        int_type *output, const int_type &left,
                        const int_type &right) noexcept {
                      ZoneScopedN("flow::integer_type::binary");
                      std::construct_at(output, operation(left, right));
                    });
    return result;
  }

  template <typename Operation>
  static integer_resource unary(const integer_resource &value,
                                Operation operation) noexcept {
    auto result = flow::resource<int_type>();
    flow::construct(result, flow::read(value.m_runtime),
                    [operation = std::move(operation)](
                        int_type *output, const int_type &input) noexcept {
                      ZoneScopedN("flow::integer_type::unary");
                      std::construct_at(output, operation(input));
                    });
    return result;
  }

  template <typename Operation>
  static flow::Resource<bool> compare(const integer_resource &lhs,
                                      const integer_resource &rhs,
                                      Operation operation) noexcept {
    auto result = flow::resource<bool>();
    flow::construct(
        result, flow::read(lhs.m_runtime), flow::read(rhs.m_runtime),
        [operation = std::move(operation)](bool *output, const int_type &left,
                                           const int_type &right) noexcept {
          ZoneScopedN("flow::integer_type::compare");
          std::construct_at(output, operation(left, right));
        });
    return result;
  }

  template <typename Operation>
  static flow::Resource<bool> predicate(const integer_resource &value,
                                        Operation operation) noexcept {
    auto result = flow::resource<bool>();
    flow::construct(result, flow::read(value.m_runtime),
                    [operation = std::move(operation)](
                        bool *output, const int_type &input) noexcept {
                      ZoneScopedN("flow::integer_type::predicate");
                      std::construct_at(output, operation(input));
                    });
    return result;
  }

  flow::Resource<int_type> m_runtime;
};

} // namespace detail

using u8 = detail::integer_resource<std::uint8_t>;
using u16 = detail::integer_resource<std::uint16_t>;
using u32 = detail::integer_resource<std::uint32_t>;
using u64 = detail::integer_resource<std::uint64_t>;

using i8 = detail::integer_resource<std::int8_t>;
using i16 = detail::integer_resource<std::int16_t>;
using i32 = detail::integer_resource<std::int32_t>;
using i64 = detail::integer_resource<std::int64_t>;

} // namespace strobe::flow
