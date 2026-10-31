#include <strobe/core/lina/quaternion.hpp>

#include <gtest/gtest.h>

#include <cmath>
#include <numbers>

namespace {

constexpr float epsilon = 1e-5F;

void expect_quaternion_near(const strobe::quaternion &actual,
                            const strobe::quaternion &expected,
                            float tolerance = epsilon) {
  EXPECT_NEAR(actual.x(), expected.x(), tolerance);
  EXPECT_NEAR(actual.y(), expected.y(), tolerance);
  EXPECT_NEAR(actual.z(), expected.z(), tolerance);
  EXPECT_NEAR(actual.w(), expected.w(), tolerance);
}

void expect_vec3_near(const strobe::vec3 &actual,
                      const strobe::vec3 &expected,
                      float tolerance = epsilon) {
  EXPECT_NEAR(actual.x(), expected.x(), tolerance);
  EXPECT_NEAR(actual.y(), expected.y(), tolerance);
  EXPECT_NEAR(actual.z(), expected.z(), tolerance);
}

} // namespace

TEST(Quaternion, identity_construction_and_component_access) {
  strobe::quaternion value;
  EXPECT_EQ(value, strobe::quaternion::identity());
  EXPECT_FLOAT_EQ(value.x(), 0.0F);
  EXPECT_FLOAT_EQ(value.y(), 0.0F);
  EXPECT_FLOAT_EQ(value.z(), 0.0F);
  EXPECT_FLOAT_EQ(value.w(), 1.0F);

  value[0] = 1.0F;
  value.y() = 2.0F;
  value.z() = 3.0F;
  value.w() = 4.0F;

  const strobe::quaternion copy = value;
  EXPECT_FLOAT_EQ(copy.x(), 1.0F);
  EXPECT_FLOAT_EQ(copy[1], 2.0F);
  EXPECT_FLOAT_EQ(copy[2], 3.0F);
  EXPECT_FLOAT_EQ(copy[3], 4.0F);
}

TEST(Quaternion, axis_angle_rotation_rotates_vectors) {
  const auto quarter_turn = strobe::quaternion::rotation(
      strobe::vec3(0.0F, 0.0F, 1.0F), std::numbers::pi_v<float> / 2.0F);

  expect_vec3_near(quarter_turn * strobe::vec3(1.0F, 0.0F, 0.0F),
                   strobe::vec3(0.0F, 1.0F, 0.0F));
  EXPECT_NEAR(quarter_turn.angle(), std::numbers::pi_v<float> / 2.0F,
              epsilon);
  expect_vec3_near(quarter_turn.axis(), strobe::vec3(0.0F, 0.0F, 1.0F));

  EXPECT_EQ(strobe::quaternion::rotation(strobe::vec3(0.0F), 1.0F),
            strobe::quaternion::identity());
}

TEST(Quaternion, rotation_between_handles_general_parallel_and_opposite_vectors) {
  const strobe::vec3 x(1.0F, 0.0F, 0.0F);
  const strobe::vec3 y(0.0F, 1.0F, 0.0F);

  expect_vec3_near(strobe::quaternion::rotation_between(x, y) * x, y);
  EXPECT_EQ(strobe::quaternion::rotation_between(x, x),
            strobe::quaternion::identity());
  expect_vec3_near(strobe::quaternion::rotation_between(x, -x) * x, -x);
}

TEST(Quaternion, euler_rotation_matches_axis_angle_for_single_axes) {
  const float angle = std::numbers::pi_v<float> / 3.0F;

  const auto pitch = strobe::quaternion::from_euler({angle, 0.0F, 0.0F});
  const auto yaw = strobe::quaternion::from_euler({0.0F, angle, 0.0F});
  const auto roll = strobe::quaternion::from_euler({0.0F, 0.0F, angle});

  expect_quaternion_near(
      pitch, strobe::quaternion::rotation({1.0F, 0.0F, 0.0F}, angle));
  expect_quaternion_near(
      yaw, strobe::quaternion::rotation({0.0F, 1.0F, 0.0F}, angle));
  expect_quaternion_near(
      roll, strobe::quaternion::rotation({0.0F, 0.0F, 1.0F}, angle));
}

TEST(Quaternion, arithmetic_length_conjugate_and_inverse) {
  const strobe::quaternion value(1.0F, -2.0F, 3.0F, -4.0F);
  EXPECT_FLOAT_EQ(value.length2(), 30.0F);
  EXPECT_FLOAT_EQ(strobe::length2(value), 30.0F);
  EXPECT_NEAR(value.length(), std::sqrt(30.0F), epsilon);
  EXPECT_NEAR(strobe::length(value), std::sqrt(30.0F), epsilon);
  EXPECT_EQ(strobe::conjugate(value),
            strobe::quaternion(-1.0F, 2.0F, -3.0F, -4.0F));
  EXPECT_FLOAT_EQ(strobe::dot(value, value), 30.0F);

  const auto normalized = strobe::normalize(value);
  EXPECT_NEAR(strobe::length(normalized), 1.0F, epsilon);

  const auto product = value * strobe::inverse(value);
  expect_quaternion_near(product, strobe::quaternion::identity());
}

TEST(Quaternion, multiplication_composes_rotations) {
  const auto x_rotation = strobe::quaternion::rotation(
      {1.0F, 0.0F, 0.0F}, std::numbers::pi_v<float> / 2.0F);
  const auto z_rotation = strobe::quaternion::rotation(
      {0.0F, 0.0F, 1.0F}, std::numbers::pi_v<float> / 2.0F);
  const strobe::vec3 value(0.0F, 1.0F, 0.0F);

  expect_vec3_near((z_rotation * x_rotation) * value,
                   z_rotation * (x_rotation * value));

  auto compound = z_rotation;
  compound *= x_rotation;
  expect_quaternion_near(compound, z_rotation * x_rotation);
}

TEST(Quaternion, interpolation_preserves_endpoints_and_shortest_path) {
  const auto start = strobe::quaternion::identity();
  const auto end = strobe::quaternion::rotation(
      {0.0F, 0.0F, 1.0F}, std::numbers::pi_v<float> / 2.0F);

  expect_quaternion_near(strobe::lerp(start, end, 0.0F), start);
  expect_quaternion_near(strobe::nlerp(start, end, 0.0F), start);
  expect_quaternion_near(strobe::slerp(start, end, 0.0F), start);

  const auto halfway = strobe::slerp(start, end, 0.5F);
  const float root_half = std::sqrt(0.5F);
  expect_vec3_near(halfway * strobe::vec3(1.0F, 0.0F, 0.0F),
                   strobe::vec3(root_half, root_half, 0.0F));

  const auto same_rotation = strobe::nlerp(start, -start, 0.5F);
  expect_quaternion_near(same_rotation, start);
}

TEST(Quaternion, exponential_logarithm_power_and_square_root_round_trip) {
  const strobe::quaternion value(0.2F, -0.3F, 0.4F, 0.5F);
  expect_quaternion_near(strobe::exp(strobe::log(value)), value, 2e-5F);

  const auto square_root = strobe::sqrt(value);
  expect_quaternion_near(square_root * square_root, value, 2e-5F);
  expect_quaternion_near(strobe::pow(value, 1.0F), value, 2e-5F);
}

TEST(Quaternion, component_comparisons_and_absolute_value) {
  const strobe::quaternion lhs(-1.0F, 2.0F, 3.0F, -4.0F);
  const strobe::quaternion rhs(0.0F, 2.0F, 1.0F, 5.0F);

  const auto less = lhs < rhs;
  const auto less_equal = lhs <= rhs;
  const auto greater = lhs > rhs;
  const auto greater_equal = lhs >= rhs;

  EXPECT_TRUE(less[0]);
  EXPECT_FALSE(less[1]);
  EXPECT_FALSE(less[2]);
  EXPECT_TRUE(less[3]);
  EXPECT_TRUE(less_equal[0]);
  EXPECT_TRUE(less_equal[1]);
  EXPECT_FALSE(greater[0]);
  EXPECT_FALSE(greater[1]);
  EXPECT_TRUE(greater[2]);
  EXPECT_FALSE(greater[3]);
  EXPECT_FALSE(greater_equal[0]);
  EXPECT_TRUE(greater_equal[1]);
  EXPECT_TRUE(greater_equal[2]);
  EXPECT_FALSE(greater_equal[3]);

  EXPECT_EQ(strobe::abs(lhs), strobe::quaternion(1.0F, 2.0F, 3.0F, 4.0F));
}

TEST(Quaternion, rotations_preserve_vector_length_across_axes_and_angles) {
  const strobe::vec3 axes[] = {
      {1.0F, 2.0F, 3.0F},
      {-4.0F, 1.0F, 2.0F},
      {0.0F, -3.0F, 5.0F},
  };
  const float angles[] = {-2.4F, -0.3F, 0.0F, 0.7F, 2.8F};
  const strobe::vec3 vectors[] = {
      {1.0F, 0.0F, 0.0F},
      {-2.0F, 5.0F, 1.0F},
      {0.25F, -0.5F, 4.0F},
  };

  for (const auto &axis : axes) {
    for (const float angle : angles) {
      const auto rotation = strobe::quaternion::rotation(axis, angle);
      EXPECT_NEAR(strobe::length(rotation), 1.0F, epsilon);
      for (const auto &vector : vectors) {
        EXPECT_NEAR(strobe::length(rotation * vector), strobe::length(vector),
                    2e-5F);
      }
    }
  }
}

TEST(Quaternion, inverse_undoes_multiple_non_unit_quaternions) {
  const strobe::quaternion values[] = {
      {1.0F, 2.0F, 3.0F, 4.0F},
      {-0.5F, 0.25F, 2.0F, -1.0F},
      {0.0F, 0.0F, 0.0F, 3.0F},
  };

  for (const auto &value : values) {
    expect_quaternion_near(value * strobe::inverse(value),
                           strobe::quaternion::identity(), 2e-5F);
    expect_quaternion_near(strobe::inverse(value) * value,
                           strobe::quaternion::identity(), 2e-5F);
  }
}

TEST(Quaternion, general_euler_rotation_matches_documented_composition) {
  const strobe::vec3 angles(0.37F, -0.81F, 1.14F);
  const auto pitch = strobe::quaternion::rotation(
      {1.0F, 0.0F, 0.0F}, angles.x());
  const auto yaw = strobe::quaternion::rotation(
      {0.0F, 1.0F, 0.0F}, angles.y());
  const auto roll = strobe::quaternion::rotation(
      {0.0F, 0.0F, 1.0F}, angles.z());

  expect_quaternion_near(strobe::quaternion::from_euler(angles),
                         yaw * pitch * roll);
}

TEST(Quaternion, rotation_between_accepts_non_unit_inputs) {
  const strobe::vec3 from(4.0F, -2.0F, 1.0F);
  const strobe::vec3 to(-3.0F, 5.0F, 2.0F);
  const auto rotation = strobe::quaternion::rotation_between(from, to);

  expect_vec3_near(strobe::normalize(rotation * from), strobe::normalize(to),
                   2e-5F);
  EXPECT_NEAR(strobe::length(rotation), 1.0F, epsilon);
}

TEST(Quaternion, interpolation_stays_normalized_and_reaches_rotations) {
  const auto start = strobe::quaternion::rotation(
      {1.0F, 2.0F, -1.0F}, -0.6F);
  const auto end = strobe::quaternion::rotation(
      {-2.0F, 1.0F, 3.0F}, 1.7F);
  const strobe::vec3 probe(0.3F, -1.2F, 2.5F);

  for (const float t : {0.0F, 0.1F, 0.35F, 0.5F, 0.9F, 1.0F}) {
    EXPECT_NEAR(strobe::length(strobe::nlerp(start, end, t)), 1.0F,
                epsilon);
    EXPECT_NEAR(strobe::length(strobe::slerp(start, end, t)), 1.0F,
                epsilon);
  }

  expect_vec3_near(strobe::slerp(start, end, 0.0F) * probe, start * probe);
  expect_vec3_near(strobe::slerp(start, end, 1.0F) * probe, end * probe);
  expect_vec3_near(strobe::nlerp(start, -end, 1.0F) * probe, end * probe);
}

TEST(Quaternion, logarithm_and_exponential_round_trip_multiple_values) {
  const strobe::quaternion values[] = {
      {0.2F, -0.3F, 0.4F, 0.5F},
      {-1.0F, 2.0F, 0.5F, 3.0F},
      {0.0F, 0.0F, 0.0F, 2.5F},
  };

  for (const auto &value : values) {
    expect_quaternion_near(strobe::exp(strobe::log(value)), value, 3e-5F);
  }
}

TEST(Quaternion, slerp_is_stable_across_linear_fallback_boundary) {
  const auto start = strobe::quaternion::identity();
  const auto close = strobe::quaternion::rotation(
      {0.0F, 1.0F, 0.0F}, 0.05F);
  const auto farther = strobe::quaternion::rotation(
      {0.0F, 1.0F, 0.0F}, 0.08F);

  for (const auto &end : {close, farther}) {
    for (const float t : {0.0F, 0.25F, 0.5F, 0.75F, 1.0F}) {
      const auto interpolated = strobe::slerp(start, end, t);
      EXPECT_NEAR(strobe::length(interpolated), 1.0F, epsilon);
      EXPECT_NEAR(interpolated.angle(), end.angle() * t, 2e-5F);
      if (t > 0.0F) {
        expect_vec3_near(interpolated.axis(),
                         strobe::vec3(0.0F, 1.0F, 0.0F), 2e-5F);
      }
    }
  }
}

TEST(Quaternion, inverse_reverses_composition) {
  const auto a = strobe::quaternion::rotation({1.0F, 2.0F, 3.0F}, 0.71F);
  const auto b = strobe::quaternion::rotation({-2.0F, 1.0F, 0.5F}, -1.13F);

  expect_quaternion_near(strobe::inverse(a * b),
                         strobe::inverse(b) * strobe::inverse(a), 2e-5F);
}

TEST(Quaternion, double_precision_rotation_round_trip) {
  using quaterniond = strobe::detail::quaternion<double>;
  const auto rotation = quaterniond::rotation({1.0, -2.0, 3.0}, 1.23456789);
  const strobe::dvec3 value(0.25, -4.0, 2.5);
  const auto restored = strobe::inverse(rotation) * (rotation * value);

  EXPECT_NEAR(restored.x(), value.x(), 1e-12);
  EXPECT_NEAR(restored.y(), value.y(), 1e-12);
  EXPECT_NEAR(restored.z(), value.z(), 1e-12);
}
