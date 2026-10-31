#include <strobe/core/lina/vec.hpp>

#include <gtest/gtest.h>
#include <cmath>
#include <cstdint>
#include <tuple>
#include <type_traits>

TEST(Vec, construction_and_numeric_conversion) {
  const strobe::vec3 zero;
  EXPECT_EQ(zero, strobe::vec3(0.0F));

  const strobe::vec3 uniform(2.0F);
  EXPECT_EQ(uniform, strobe::vec3(2.0F, 2.0F, 2.0F));

  const strobe::ivec3 integers(1, -2, 3);
  const strobe::vec3 floats = integers;
  const strobe::dvec3 doubles = floats;
  EXPECT_EQ(floats, strobe::vec3(1.0F, -2.0F, 3.0F));
  EXPECT_EQ(doubles, strobe::dvec3(1.0, -2.0, 3.0));
}

TEST(Vec, indexed_and_named_access) {
  strobe::vec4 value(1.0F, 2.0F, 3.0F, 4.0F);

  value[0] = 5.0F;
  value.y() = 6.0F;
  value.b() = 7.0F;
  value.a() = 8.0F;

  EXPECT_FLOAT_EQ(value.x(), 5.0F);
  EXPECT_FLOAT_EQ(value.g(), 6.0F);
  EXPECT_FLOAT_EQ(value.z(), 7.0F);
  EXPECT_FLOAT_EQ(value.w(), 8.0F);

  const strobe::vec4 const_value = value;
  EXPECT_FLOAT_EQ(const_value.r(), 5.0F);
  EXPECT_FLOAT_EQ(const_value[1], 6.0F);
}

TEST(Vec, coordinate_and_color_swizzles) {
  const strobe::vec4 value(1.0F, 2.0F, 3.0F, 4.0F);

  EXPECT_EQ(value.xy(), strobe::vec2(1.0F, 2.0F));
  EXPECT_EQ(value.xz(), strobe::vec2(1.0F, 3.0F));
  EXPECT_EQ(value.xw(), strobe::vec2(1.0F, 4.0F));
  EXPECT_EQ(value.yz(), strobe::vec2(2.0F, 3.0F));
  EXPECT_EQ(value.yw(), strobe::vec2(2.0F, 4.0F));
  EXPECT_EQ(value.zw(), strobe::vec2(3.0F, 4.0F));
  EXPECT_EQ(value.xyz(), strobe::vec3(1.0F, 2.0F, 3.0F));
  EXPECT_EQ(value.xyw(), strobe::vec3(1.0F, 2.0F, 4.0F));
  EXPECT_EQ(value.xzw(), strobe::vec3(1.0F, 3.0F, 4.0F));
  EXPECT_EQ(value.yzw(), strobe::vec3(2.0F, 3.0F, 4.0F));
  EXPECT_EQ(value.xyzw(), value);

  EXPECT_EQ(value.rg(), value.xy());
  EXPECT_EQ(value.rb(), value.xz());
  EXPECT_EQ(value.ra(), value.xw());
  EXPECT_EQ(value.gb(), value.yz());
  EXPECT_EQ(value.ga(), value.yw());
  EXPECT_EQ(value.ba(), value.zw());
  EXPECT_EQ(value.rgb(), value.xyz());
  EXPECT_EQ(value.rga(), value.xyw());
  EXPECT_EQ(value.rba(), value.xzw());
  EXPECT_EQ(value.gba(), value.yzw());
  EXPECT_EQ(value.rgba(), value);
}

TEST(Vec, tuple_access) {
  strobe::vec3 value(1.0F, 2.0F, 3.0F);
  static_assert(std::tuple_size_v<strobe::vec3> == 3);
  static_assert(std::is_same_v<std::tuple_element_t<1, strobe::vec3>, float>);

  std::get<1>(value) = 5.0F;
  EXPECT_FLOAT_EQ(std::get<0>(value), 1.0F);
  EXPECT_FLOAT_EQ(std::get<1>(value), 5.0F);
  EXPECT_FLOAT_EQ(std::get<2>(std::move(value)), 3.0F);
}

TEST(Vec, unary_and_componentwise_arithmetic) {
  const strobe::vec3 lhs(2.0F, 4.0F, 8.0F);
  const strobe::vec3 rhs(1.0F, 2.0F, 4.0F);

  EXPECT_EQ(+lhs, lhs);
  EXPECT_EQ(-lhs, strobe::vec3(-2.0F, -4.0F, -8.0F));
  EXPECT_EQ(lhs + rhs, strobe::vec3(3.0F, 6.0F, 12.0F));
  EXPECT_EQ(lhs - rhs, strobe::vec3(1.0F, 2.0F, 4.0F));
  EXPECT_EQ(lhs * rhs, strobe::vec3(2.0F, 8.0F, 32.0F));
  EXPECT_EQ(lhs / rhs, strobe::vec3(2.0F, 2.0F, 2.0F));
}

TEST(Vec, scalar_arithmetic_in_both_directions) {
  const strobe::vec3 value(2.0F, 4.0F, 8.0F);

  EXPECT_EQ(value + 2, strobe::vec3(4.0F, 6.0F, 10.0F));
  EXPECT_EQ(value - 2, strobe::vec3(0.0F, 2.0F, 6.0F));
  EXPECT_EQ(value * 2, strobe::vec3(4.0F, 8.0F, 16.0F));
  EXPECT_EQ(value / 2, strobe::vec3(1.0F, 2.0F, 4.0F));
  EXPECT_EQ(2 + value, strobe::vec3(4.0F, 6.0F, 10.0F));
  EXPECT_EQ(10 - value, strobe::vec3(8.0F, 6.0F, 2.0F));
  EXPECT_EQ(2 * value, strobe::vec3(4.0F, 8.0F, 16.0F));
  EXPECT_EQ(16 / value, strobe::vec3(8.0F, 4.0F, 2.0F));
}

TEST(Vec, unsigned_vectors_accept_integer_literals) {
  const strobe::uvec2 extent(17U, 9U);
  constexpr int workgroup_size = 8;
  const strobe::uvec2 count =
      (extent + workgroup_size - 1) / workgroup_size;

  EXPECT_EQ(count, strobe::uvec2(3U, 2U));
}

TEST(Vec, compound_arithmetic) {
  strobe::vec3 value(2.0F, 4.0F, 8.0F);
  value += strobe::vec3(1.0F, 2.0F, 4.0F);
  value -= strobe::vec3(1.0F, 1.0F, 1.0F);
  value *= strobe::vec3(2.0F, 2.0F, 2.0F);
  value /= strobe::vec3(2.0F, 5.0F, 11.0F);
  EXPECT_EQ(value, strobe::vec3(2.0F, 2.0F, 2.0F));

  value += 1.0F;
  value -= 2.0F;
  value *= 4.0F;
  value /= 2.0F;
  EXPECT_EQ(value, strobe::vec3(2.0F, 2.0F, 2.0F));
}

TEST(Vec, relational_operators_return_component_masks) {
  const strobe::ivec3 lhs(1, 2, 3);
  const strobe::ivec3 rhs(2, 2, 1);

  const auto less = lhs < rhs;
  const auto less_equal = lhs <= rhs;
  const auto greater = lhs > rhs;
  const auto greater_equal = lhs >= rhs;

  EXPECT_TRUE(less[0]);
  EXPECT_FALSE(less[1]);
  EXPECT_FALSE(less[2]);
  EXPECT_TRUE(less_equal[0]);
  EXPECT_TRUE(less_equal[1]);
  EXPECT_FALSE(less_equal[2]);
  EXPECT_FALSE(greater[0]);
  EXPECT_FALSE(greater[1]);
  EXPECT_TRUE(greater[2]);
  EXPECT_FALSE(greater_equal[0]);
  EXPECT_TRUE(greater_equal[1]);
  EXPECT_TRUE(greater_equal[2]);
}

TEST(Vec, length_dot_and_normalization) {
  const strobe::vec3 value(3.0F, 4.0F, 0.0F);

  EXPECT_FLOAT_EQ(value.length2(), 25.0F);
  EXPECT_FLOAT_EQ(strobe::length2(value), 25.0F);
  EXPECT_FLOAT_EQ(value.length(), 5.0F);
  EXPECT_FLOAT_EQ(strobe::length(value), 5.0F);
  EXPECT_FLOAT_EQ(strobe::dot(value, strobe::vec3(2.0F, 1.0F, 3.0F)),
                  10.0F);

  const strobe::vec3 normalized = strobe::normalize(value);
  EXPECT_NEAR(normalized.x(), 0.6F, 1e-6F);
  EXPECT_NEAR(normalized.y(), 0.8F, 1e-6F);
  EXPECT_NEAR(normalized.z(), 0.0F, 1e-6F);
  EXPECT_NEAR(strobe::length(normalized), 1.0F, 1e-6F);
}

TEST(Vec, cross_product) {
  const strobe::vec3 x(1.0F, 0.0F, 0.0F);
  const strobe::vec3 y(0.0F, 1.0F, 0.0F);

  EXPECT_EQ(strobe::cross(x, y), strobe::vec3(0.0F, 0.0F, 1.0F));
  EXPECT_EQ(strobe::cross(y, x), strobe::vec3(0.0F, 0.0F, -1.0F));
}

TEST(Vec, min_max_abs_and_clamp) {
  const strobe::vec3 value(-2.0F, 3.0F, 8.0F);
  const strobe::vec3 other(-4.0F, 5.0F, 6.0F);

  EXPECT_EQ(strobe::min(value, other), strobe::vec3(-4.0F, 3.0F, 6.0F));
  EXPECT_EQ(strobe::min(value, 1.0F), strobe::vec3(-2.0F, 1.0F, 1.0F));
  EXPECT_EQ(strobe::max(value, other), strobe::vec3(-2.0F, 5.0F, 8.0F));
  EXPECT_EQ(strobe::max(value, 1.0F), strobe::vec3(1.0F, 3.0F, 8.0F));
  EXPECT_EQ(strobe::abs(value), strobe::vec3(2.0F, 3.0F, 8.0F));
  EXPECT_EQ(strobe::clamp(value, 0.0F, 5.0F),
            strobe::vec3(0.0F, 3.0F, 5.0F));
  EXPECT_EQ(strobe::clamp(value, strobe::vec3(-1.0F, 0.0F, 7.0F),
                          strobe::vec3(1.0F, 2.0F, 9.0F)),
            strobe::vec3(-1.0F, 2.0F, 8.0F));
}

TEST(Vec, lerp_reflect_and_refract) {
  EXPECT_EQ(strobe::lerp(strobe::vec3(0.0F), strobe::vec3(10.0F), 0.25F),
            strobe::vec3(2.5F));

  const strobe::vec3 incident(0.0F, -1.0F, 0.0F);
  const strobe::vec3 normal(0.0F, 1.0F, 0.0F);
  EXPECT_EQ(strobe::reflect(incident, normal),
            strobe::vec3(0.0F, 1.0F, 0.0F));
  EXPECT_EQ(strobe::refract(incident, normal, 0.5F), incident);

  const float root_three_over_two = std::sqrt(3.0F) / 2.0F;
  const strobe::vec3 shallow_incident(root_three_over_two, -0.5F, 0.0F);
  EXPECT_EQ(strobe::refract(shallow_incident, normal, 2.0F),
            strobe::vec3(0.0F));
}
