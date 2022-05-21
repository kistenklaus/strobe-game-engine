#pragma once
#include <cinttypes>

namespace strb {

// template <typename... Ts> using tuple = std::tuple<Ts...>;

template <typename T0, typename T1> struct tuple2 {
private:
  T0 t0;
  T1 t1;

public:
  constexpr tuple2(T0 t0, T1 t1) : t0(t0), t1(t1) {}
  constexpr T0 &get0() { return t0; }
  constexpr T1 &get1() { return t1; }
  constexpr const T0 &get0() const { return t0; }
  constexpr const T1 &get1() const { return t1; }
};

template <typename T0, typename T1, typename T2> struct tuple3 {
private:
  T0 t0;
  T1 t1;
  T2 t2;

public:
  constexpr tuple3(T0 t0, T1 t1, T2 t2) : t0(t0), t1(t1), t2(t2) {}
  constexpr T0 &get0() { return t0; }
  constexpr T1 &get1() { return t1; }
  constexpr T2 &get2() { return t2; }
  constexpr const T0 &get0() const { return t0; }
  constexpr const T1 &get1() const { return t1; }
  constexpr const T2 &get2() const { return t2; }
};

template <typename T0, typename T1, typename T2, typename T3> struct tuple4 {
private:
  T0 t0;
  T1 t1;
  T2 t2;
  T3 t3;

public:
  tuple4(T0 t0, T1 t1, T2 t2, T3 t3) : t0(t0), t1(t1), t2(t2), t3(t3) {}
  constexpr T0 &get0() { return t0; }
  constexpr T1 &get1() { return t1; }
  constexpr T2 &get2() { return t2; }
  constexpr T3 &get3() { return t3; }
  constexpr const T0 &get0() const { return t0; }
  constexpr const T1 &get1() const { return t1; }
  constexpr const T2 &get2() const { return t2; }
  constexpr const T3 &get3() const { return t3; }
};

template <typename T0, typename T1, typename T2, typename T3, typename T4>
struct tuple5 {
private:
  T0 t0;
  T1 t1;
  T2 t2;
  T3 t3;
  T4 t4;

public:
  tuple5(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4)
      : t0(t0), t1(t1), t2(t2), t3(t3), t4(t4) {}
  constexpr T0 &get0() { return t0; }
  constexpr T1 &get1() { return t1; }
  constexpr T2 &get2() { return t2; }
  constexpr T3 &get3() { return t3; }
  constexpr T4 &get4() { return t4; }
  constexpr const T0 &get0() const { return t0; }
  constexpr const T1 &get1() const { return t1; }
  constexpr const T2 &get2() const { return t2; }
  constexpr const T3 &get3() const { return t3; }
  constexpr const T4 &get4() const { return t4; }
};

using tuple3i = tuple3<uint32_t, uint32_t, uint32_t>;
using tuple3f = tuple3<float, float, float>;
using tuple4i = tuple4<uint32_t, uint32_t, uint32_t, uint32_t>;
using tuple4f = tuple4<float, float, float, float>;
using tuple5i = tuple5<uint32_t, uint32_t, uint32_t, uint32_t, uint32_t>;
using tuple5f = tuple5<float, float, float, float, float>;

} // namespace strb
