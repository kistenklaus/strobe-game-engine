#pragma once

#include <cstddef>
#include <variant>
namespace strobe {

template <typename V, typename E>
struct Result {
  static constexpr Result<V, E> Err(E&& e) {
    return Result(std::move(e));
  }
  static constexpr Result<V, E> Err(const E& e) {
    return Result(e);
  }
  static constexpr Result<V, E> Ok(const V& v) {
    return Result(v);
  }
  static constexpr Result<V, E> Ok(V&& v) {
    return Result(std::move(v));
  }
  Result(const Result&) = default;
  Result(Result&&) = default;
  Result& operator=(const Result&) = default;
  Result& operator=(Result&&) = default;
  ~Result() {
    if (m_succ) {
      m_ok.~V();
    }else {
      m_err.~E();
    }
  }

  inline operator bool() const { return m_succ;}
  inline V&& unwrap() { 
    return std::move(m_ok);
  }
  inline E& error() { return m_err;}
  inline constexpr const E& error() const { return m_err;}

 private:
  constexpr explicit Result(V&& v) : m_ok(std::move(v)), m_succ(true) { }
  constexpr explicit Result(const V& v) : m_ok(v), m_succ(true) { }
  constexpr explicit Result(E&& e) : m_err(std::move(e)), m_succ(false) { }
  constexpr explicit Result(const E& e) : m_err(e), m_succ(false) { }
  union {
    V m_ok;
    E m_err;
  };
  bool m_succ;
};

}  // namespace strobe
