#include <strobe/core/lina/mat.hpp>

#include <gtest/gtest.h>

#include <cmath>
#include <numbers>

namespace {

constexpr float epsilon = 1e-5F;

template <uint32_t N, uint32_t M>
void expect_matrix_near(const strobe::detail::mat<N, M, float> &actual,
                        const strobe::detail::mat<N, M, float> &expected,
                        float tolerance = epsilon) {
  for (uint32_t row = 0; row < N; ++row) {
    for (uint32_t column = 0; column < M; ++column) {
      EXPECT_NEAR(actual.at(row, column), expected.at(row, column), tolerance)
          << "at (" << row << ", " << column << ')';
    }
  }
}

template <uint32_t N>
void expect_identity_near(const strobe::detail::mat<N, N, float> &actual,
                          float tolerance = epsilon) {
  expect_matrix_near(actual, strobe::detail::mat<N, N, float>::identity(),
                     tolerance);
}

void expect_vec4_near(const strobe::vec4 &actual,
                      const strobe::vec4 &expected,
                      float tolerance = epsilon) {
  for (uint32_t i = 0; i < 4; ++i) {
    EXPECT_NEAR(actual[i], expected[i], tolerance) << "at index " << i;
  }
}

} // namespace

TEST(Mat, construction_identity_and_access) {
  const auto identity = strobe::mat3::identity();
  for (uint32_t row = 0; row < 3; ++row) {
    for (uint32_t column = 0; column < 3; ++column) {
      EXPECT_FLOAT_EQ(identity.at(row, column), row == column ? 1.0F : 0.0F);
    }
  }

  strobe::mat3 value({1.0F, 2.0F, 3.0F},
                     {4.0F, 5.0F, 6.0F},
                     {7.0F, 8.0F, 9.0F});
  EXPECT_EQ(value.column(1), strobe::vec3(4.0F, 5.0F, 6.0F));
  EXPECT_EQ(value.row(1), strobe::vec3(2.0F, 5.0F, 8.0F));
  value[2][0] = 10.0F;
  EXPECT_FLOAT_EQ(value.at(0, 2), 10.0F);

  const strobe::dmat3 double_value = value;
  EXPECT_DOUBLE_EQ(double_value.at(0, 2), 10.0);
}

TEST(Mat, arithmetic_and_elementwise_operations) {
  const strobe::mat2 lhs({1.0F, -2.0F}, {3.0F, -4.0F});
  const strobe::mat2 rhs({5.0F, 6.0F}, {7.0F, 8.0F});

  EXPECT_EQ(lhs + rhs, strobe::mat2({6.0F, 4.0F}, {10.0F, 4.0F}));
  EXPECT_EQ(lhs - rhs, strobe::mat2({-4.0F, -8.0F}, {-4.0F, -12.0F}));
  EXPECT_EQ(lhs + 2.0F, strobe::mat2({3.0F, 0.0F}, {5.0F, -2.0F}));
  EXPECT_EQ(2.0F - lhs, strobe::mat2({1.0F, 4.0F}, {-1.0F, 6.0F}));
  EXPECT_EQ(lhs * 2.0F, strobe::mat2({2.0F, -4.0F}, {6.0F, -8.0F}));
  EXPECT_EQ(strobe::abs(lhs), strobe::mat2({1.0F, 2.0F}, {3.0F, 4.0F}));
  EXPECT_EQ(strobe::elementwise_mul(lhs, rhs),
            strobe::mat2({5.0F, -12.0F}, {21.0F, -32.0F}));
  EXPECT_EQ(strobe::lerp(lhs, rhs, 0.25F),
            strobe::mat2({2.0F, 0.0F}, {4.0F, -1.0F}));
}

TEST(Mat, matrix_vector_and_square_matrix_multiplication) {
  const strobe::mat3 lhs({1.0F, 4.0F, 7.0F},
                         {2.0F, 5.0F, 8.0F},
                         {3.0F, 6.0F, 9.0F});
  EXPECT_EQ(lhs * strobe::vec3(1.0F, 2.0F, 3.0F),
            strobe::vec3(14.0F, 32.0F, 50.0F));

  const strobe::mat3 rhs({9.0F, 6.0F, 3.0F},
                         {8.0F, 5.0F, 2.0F},
                         {7.0F, 4.0F, 1.0F});
  const strobe::mat3 expected({30.0F, 84.0F, 138.0F},
                              {24.0F, 69.0F, 114.0F},
                              {18.0F, 54.0F, 90.0F});
  EXPECT_EQ(lhs * rhs, expected);

  auto compound = lhs;
  compound *= rhs;
  EXPECT_EQ(compound, expected);
}

TEST(Mat, transpose_and_outer_product) {
  const strobe::mat3 value({1.0F, 4.0F, 7.0F},
                           {2.0F, 5.0F, 8.0F},
                           {3.0F, 6.0F, 9.0F});
  const strobe::mat3 expected({1.0F, 2.0F, 3.0F},
                              {4.0F, 5.0F, 6.0F},
                              {7.0F, 8.0F, 9.0F});
  EXPECT_EQ(strobe::transpose(value), expected);
  EXPECT_EQ(strobe::transpose(strobe::transpose(value)), value);

  const auto outer = strobe::outer_product(strobe::vec3(1.0F, 2.0F, 3.0F),
                                            strobe::vec2(4.0F, 5.0F));
  EXPECT_EQ(outer.column(0), strobe::vec3(4.0F, 8.0F, 12.0F));
  EXPECT_EQ(outer.column(1), strobe::vec3(5.0F, 10.0F, 15.0F));
}

TEST(Mat, rectangular_transpose_swaps_rows_and_columns) {
  const strobe::mat2x3 value({1.0F, 2.0F, 3.0F},
                             {4.0F, 5.0F, 6.0F});
  const strobe::mat3x2 expected({1.0F, 4.0F},
                                {2.0F, 5.0F},
                                {3.0F, 6.0F});

  EXPECT_EQ(strobe::transpose(value), expected);
  EXPECT_EQ(strobe::transpose(strobe::transpose(value)), value);
}

TEST(Mat, rectangular_matrix_multiplication_uses_result_dimensions) {
  const strobe::detail::mat<2, 3, float> lhs(
      strobe::vec2(1.0F, 4.0F),
      strobe::vec2(2.0F, 5.0F),
      strobe::vec2(3.0F, 6.0F));
  const strobe::detail::mat<3, 4, float> rhs(
      strobe::vec3(1.0F, 0.0F, 1.0F),
      strobe::vec3(0.0F, 1.0F, 1.0F),
      strobe::vec3(2.0F, 1.0F, 0.0F),
      strobe::vec3(-1.0F, 2.0F, 1.0F));
  const strobe::detail::mat<2, 4, float> expected(
      strobe::vec2(4.0F, 10.0F),
      strobe::vec2(5.0F, 11.0F),
      strobe::vec2(4.0F, 13.0F),
      strobe::vec2(6.0F, 12.0F));

  EXPECT_EQ(lhs * rhs, expected);
}

TEST(Mat, determinants_match_known_values) {
  strobe::detail::mat<1, 1, float> one;
  one.at(0, 0) = 4.0F;
  EXPECT_FLOAT_EQ(strobe::det(one), 4.0F);

  const strobe::mat2 two({4.0F, 2.0F}, {7.0F, 6.0F});
  EXPECT_FLOAT_EQ(strobe::det(two), 10.0F);

  const strobe::mat3 three({6.0F, 4.0F, 2.0F},
                           {1.0F, -2.0F, 8.0F},
                           {1.0F, 5.0F, 7.0F});
  EXPECT_FLOAT_EQ(strobe::det(three), -306.0F);

  const auto transform = strobe::mat4::translation({2.0F, 3.0F, 4.0F}) *
                         strobe::mat4::scaling(2.0F, 3.0F, 5.0F);
  EXPECT_NEAR(strobe::det(transform), 30.0F, epsilon);
}

TEST(Mat, inverses_undo_matrices_of_each_supported_size) {
  strobe::detail::mat<1, 1, float> one;
  one.at(0, 0) = 4.0F;
  expect_identity_near(one * strobe::inverse(one));

  const strobe::mat2 two({4.0F, 2.0F}, {7.0F, 6.0F});
  expect_identity_near(two * strobe::inverse(two));

  const strobe::mat3 three({3.0F, 2.0F, 0.0F},
                           {0.0F, 0.0F, 1.0F},
                           {2.0F, -2.0F, 1.0F});
  expect_identity_near(three * strobe::inverse(three));

  const auto four = strobe::mat4::transform(
      {2.0F, -3.0F, 4.0F}, {2.0F, 3.0F, 0.5F}, {0.2F, -0.7F, 1.1F});
  expect_identity_near(four * strobe::inverse(four), 3e-5F);
}

TEST(Mat, singular_inverse_returns_nan_matrix) {
  const strobe::mat3 singular({1.0F, 2.0F, 3.0F},
                              {2.0F, 4.0F, 6.0F},
                              {3.0F, 6.0F, 9.0F});
  const auto result = strobe::inverse(singular);
  for (uint32_t row = 0; row < 3; ++row) {
    for (uint32_t column = 0; column < 3; ++column) {
      EXPECT_TRUE(std::isnan(result.at(row, column)));
    }
  }
}

TEST(Mat, translation_scaling_and_rotation_transform_vectors) {
  const auto translation = strobe::mat4::translation({2.0F, -3.0F, 4.0F});
  expect_vec4_near(translation * strobe::vec4(1.0F, 2.0F, 3.0F, 1.0F),
                   strobe::vec4(3.0F, -1.0F, 7.0F, 1.0F));

  const auto scaling = strobe::mat4::scaling(2.0F, 3.0F, 4.0F);
  expect_vec4_near(scaling * strobe::vec4(1.0F, 2.0F, 3.0F, 1.0F),
                   strobe::vec4(2.0F, 6.0F, 12.0F, 1.0F));

  const auto rotation = strobe::mat4::rotation(
      {0.0F, 0.0F, 1.0F}, std::numbers::pi_v<float> / 2.0F);
  expect_vec4_near(rotation * strobe::vec4(1.0F, 0.0F, 0.0F, 0.0F),
                   strobe::vec4(0.0F, 1.0F, 0.0F, 0.0F));
}

TEST(Mat, composed_transform_matches_separate_operations) {
  const strobe::vec3 position(2.0F, -3.0F, 4.0F);
  const strobe::vec3 scale(2.0F, 3.0F, 0.5F);
  const strobe::vec3 angles(0.2F, -0.7F, 1.1F);
  const auto combined = strobe::mat4::transform(position, scale, angles);
  const auto separate = strobe::mat4::translation(position) *
                        strobe::mat4::from_euler(angles) *
                        strobe::mat4::scaling(scale.x(), scale.y(), scale.z());
  expect_matrix_near(combined, separate, 2e-5F);
}

TEST(Mat, quaternion_and_axis_angle_rotation_matrices_agree) {
  const strobe::vec3 axis(1.0F, -2.0F, 3.0F);
  const float angle = 0.83F;
  const auto quaternion = strobe::quaternion::rotation(axis, angle);

  expect_matrix_near(strobe::mat3::from_quaternion(quaternion),
                     strobe::mat3::rotation(axis, angle), 2e-5F);

  const auto transform = strobe::mat4::transform(
      {1.0F, 2.0F, 3.0F}, {2.0F, 0.5F, 4.0F}, quaternion);
  const auto expected = strobe::mat4::translation({1.0F, 2.0F, 3.0F}) *
                        strobe::mat4::from_quaternion(quaternion) *
                        strobe::mat4::scaling(2.0F, 0.5F, 4.0F);
  expect_matrix_near(transform, expected, 2e-5F);
}

TEST(Mat, look_at_places_camera_at_origin_and_target_on_negative_z) {
  const strobe::vec3 position(2.0F, 3.0F, 4.0F);
  const strobe::vec3 target(-1.0F, 1.0F, 0.0F);
  const auto view = strobe::mat4::look_at(position, target,
                                          {0.0F, 1.0F, 0.0F});

  expect_vec4_near(view * strobe::vec4(position.x(), position.y(),
                                       position.z(), 1.0F),
                   strobe::vec4(0.0F, 0.0F, 0.0F, 1.0F), 2e-5F);
  const auto transformed_target =
      view * strobe::vec4(target.x(), target.y(), target.z(), 1.0F);
  EXPECT_NEAR(transformed_target.x(), 0.0F, 2e-5F);
  EXPECT_NEAR(transformed_target.y(), 0.0F, 2e-5F);
  EXPECT_LT(transformed_target.z(), 0.0F);
}

TEST(Mat, projection_factories_map_expected_boundaries) {
  const auto perspective = strobe::mat4::perspective(
      std::numbers::pi_v<float> / 2.0F, 1.0F, 1.0F, 9.0F);
  const auto near_point = perspective * strobe::vec4(0.0F, 0.0F, -1.0F, 1.0F);
  const auto far_point = perspective * strobe::vec4(0.0F, 0.0F, -9.0F, 1.0F);
  EXPECT_NEAR(near_point.z() / near_point.w(), -1.0F, epsilon);
  EXPECT_NEAR(far_point.z() / far_point.w(), 1.0F, epsilon);

  const auto orthographic =
      strobe::mat4::ortho(-2.0F, 2.0F, -3.0F, 3.0F, 1.0F, 9.0F);
  expect_vec4_near(orthographic * strobe::vec4(-2.0F, -3.0F, -1.0F, 1.0F),
                   strobe::vec4(-1.0F, -1.0F, -1.0F, 1.0F));
  expect_vec4_near(orthographic * strobe::vec4(2.0F, 3.0F, -9.0F, 1.0F),
                   strobe::vec4(1.0F, 1.0F, 1.0F, 1.0F));
}

TEST(Mat, reverse_z_and_flipped_y_projection_options) {
  const auto perspective = strobe::mat4::perspective(
      std::numbers::pi_v<float> / 2.0F, 1.0F, 1.0F, 9.0F, true, true);
  const auto near_point = perspective * strobe::vec4(0.0F, 1.0F, -1.0F, 1.0F);
  const auto far_point = perspective * strobe::vec4(0.0F, 1.0F, -9.0F, 1.0F);
  EXPECT_NEAR(near_point.z() / near_point.w(), 1.0F, epsilon);
  EXPECT_NEAR(far_point.z() / far_point.w(), -1.0F, epsilon);
  EXPECT_LT(near_point.y() / near_point.w(), 0.0F);

  const auto orthographic = strobe::mat4::ortho(
      -2.0F, 2.0F, -3.0F, 3.0F, 1.0F, 9.0F, true, true);
  expect_vec4_near(orthographic * strobe::vec4(-2.0F, -3.0F, -1.0F, 1.0F),
                   strobe::vec4(-1.0F, 1.0F, 1.0F, 1.0F));
  expect_vec4_near(orthographic * strobe::vec4(2.0F, 3.0F, -9.0F, 1.0F),
                   strobe::vec4(1.0F, -1.0F, -1.0F, 1.0F));
}

TEST(Mat, comparison_masks_follow_column_major_element_order) {
  const strobe::mat2 lhs({1.0F, 4.0F}, {3.0F, 2.0F});
  const strobe::mat2 rhs({2.0F, 4.0F}, {1.0F, 5.0F});

  const auto less = lhs < rhs;
  const auto less_equal = lhs <= rhs;
  const auto greater = lhs > rhs;
  const auto greater_equal = lhs >= rhs;
  const auto equal = strobe::equal(lhs, rhs);
  const auto unequal = strobe::unequal(lhs, rhs);

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
  EXPECT_FALSE(equal[0]);
  EXPECT_TRUE(equal[1]);
  EXPECT_FALSE(equal[2]);
  EXPECT_FALSE(equal[3]);
  EXPECT_TRUE(unequal[0]);
  EXPECT_FALSE(unequal[1]);
  EXPECT_TRUE(unequal[2]);
  EXPECT_TRUE(unequal[3]);
}

TEST(Mat, epsilon_comparisons_include_values_at_tolerance) {
  const strobe::mat2 lhs({1.0F, 2.0F}, {3.0F, 4.0F});
  const strobe::mat2 rhs({1.01F, 2.02F}, {2.98F, 4.03F});

  const auto equal = strobe::equal(lhs, rhs, 0.02F);
  const auto unequal = strobe::unequal(lhs, rhs, 0.02F);
  EXPECT_TRUE(equal[0]);
  EXPECT_TRUE(equal[1]);
  EXPECT_TRUE(equal[2]);
  EXPECT_FALSE(equal[3]);
  EXPECT_FALSE(unequal[0]);
  EXPECT_FALSE(unequal[1]);
  EXPECT_FALSE(unequal[2]);
  EXPECT_TRUE(unequal[3]);
}

TEST(Mat, inverse_handles_row_pivoting) {
  const strobe::mat3 value({0.0F, 1.0F, 4.0F},
                           {2.0F, 0.0F, 5.0F},
                           {3.0F, 6.0F, 0.0F});
  const auto inverse = strobe::inverse(value);

  expect_identity_near(value * inverse, 2e-5F);
  expect_identity_near(inverse * value, 2e-5F);
}

TEST(Mat, adjugate_satisfies_determinant_identity) {
  const strobe::mat3 value({3.0F, 2.0F, 0.0F},
                           {0.0F, 0.0F, 1.0F},
                           {2.0F, -2.0F, 1.0F});
  const auto expected = strobe::mat3::identity() * strobe::det(value);

  expect_matrix_near(value * strobe::adjugate(value), expected, 2e-5F);
  expect_matrix_near(strobe::adjugate(value) * value, expected, 2e-5F);
}

TEST(Mat, zero_axis_rotation_is_identity) {
  EXPECT_EQ(strobe::mat3::rotation({0.0F, 0.0F, 0.0F}, 1.2F),
            strobe::mat3::identity());
  EXPECT_EQ(strobe::mat4::rotation({0.0F, 0.0F, 0.0F}, -0.7F),
            strobe::mat4::identity());
}

TEST(Mat, inverse_transpose_matches_transposed_inverse) {
  const strobe::mat3 value({2.0F, -1.0F, 4.0F},
                           {3.0F, 5.0F, 0.5F},
                           {-2.0F, 1.0F, 3.0F});

  expect_matrix_near(strobe::inverse_transpose(value),
                     strobe::transpose(strobe::inverse(value)), 2e-5F);
}

TEST(Mat, affine_inverse_matches_general_inverse) {
  const auto value = strobe::mat4::transform(
      {3.0F, -4.0F, 2.0F}, {0.5F, 3.0F, 1.5F}, {-0.4F, 0.8F, 1.2F});
  const auto affine_inverse = strobe::affine_inverse(value);

  expect_matrix_near(affine_inverse, strobe::inverse(value), 3e-5F);
  expect_identity_near(value * affine_inverse, 3e-5F);
  expect_identity_near(affine_inverse * value, 3e-5F);
}

TEST(Mat, asymmetric_projection_maps_frustum_boundaries) {
  constexpr float left = -2.0F;
  constexpr float right = 4.0F;
  constexpr float bottom = -3.0F;
  constexpr float top = 1.0F;
  constexpr float near = 2.0F;
  constexpr float far = 10.0F;
  const auto projection =
      strobe::mat4::projection(left, right, bottom, top, near, far);

  const auto near_min =
      projection * strobe::vec4(left, bottom, -near, 1.0F);
  const auto near_max =
      projection * strobe::vec4(right, top, -near, 1.0F);
  const auto far_center = projection * strobe::vec4(
      (left + right) * far / (2.0F * near),
      (bottom + top) * far / (2.0F * near), -far, 1.0F);

  EXPECT_NEAR(near_min.x() / near_min.w(), -1.0F, epsilon);
  EXPECT_NEAR(near_min.y() / near_min.w(), -1.0F, epsilon);
  EXPECT_NEAR(near_min.z() / near_min.w(), -1.0F, epsilon);
  EXPECT_NEAR(near_max.x() / near_max.w(), 1.0F, epsilon);
  EXPECT_NEAR(near_max.y() / near_max.w(), 1.0F, epsilon);
  EXPECT_NEAR(near_max.z() / near_max.w(), -1.0F, epsilon);
  EXPECT_NEAR(far_center.x() / far_center.w(), 0.0F, epsilon);
  EXPECT_NEAR(far_center.y() / far_center.w(), 0.0F, epsilon);
  EXPECT_NEAR(far_center.z() / far_center.w(), 1.0F, epsilon);
}

TEST(Mat, generic_five_by_five_determinant_and_inverse) {
  strobe::detail::mat<5, 5, float> value{};
  const float entries[5][5] = {
      {2.0F, 1.0F, 0.0F, 0.0F, 3.0F},
      {0.0F, 3.0F, 1.0F, 0.0F, 1.0F},
      {1.0F, 0.0F, 4.0F, 1.0F, 0.0F},
      {0.0F, 2.0F, 0.0F, 5.0F, 1.0F},
      {1.0F, 0.0F, 2.0F, 0.0F, 6.0F},
  };
  for (uint32_t row = 0; row < 5; ++row) {
    for (uint32_t column = 0; column < 5; ++column) {
      value.at(row, column) = entries[row][column];
    }
  }

  const auto inverse = strobe::inverse(value);
  expect_identity_near(value * inverse, 4e-5F);
  expect_identity_near(inverse * value, 4e-5F);
  EXPECT_NEAR(strobe::det(value) * strobe::det(inverse), 1.0F, 4e-5F);
}

TEST(Mat, rectangular_multiplication_supports_tall_results) {
  strobe::detail::mat<4, 2, float> lhs{};
  strobe::detail::mat<2, 3, float> rhs{};
  for (uint32_t row = 0; row < 4; ++row) {
    lhs.at(row, 0) = static_cast<float>(row + 1);
    lhs.at(row, 1) = static_cast<float>(5 - row);
  }
  rhs.at(0, 0) = 2.0F;
  rhs.at(1, 0) = -1.0F;
  rhs.at(0, 1) = 0.5F;
  rhs.at(1, 1) = 3.0F;
  rhs.at(0, 2) = -2.0F;
  rhs.at(1, 2) = 4.0F;

  const auto result = lhs * rhs;
  for (uint32_t row = 0; row < 4; ++row) {
    for (uint32_t column = 0; column < 3; ++column) {
      EXPECT_FLOAT_EQ(result.at(row, column),
                      lhs.at(row, 0) * rhs.at(0, column) +
                          lhs.at(row, 1) * rhs.at(1, column));
    }
  }
}

TEST(Mat, double_precision_transform_inverse_round_trip) {
  const auto value = strobe::dmat4::transform(
      {1.25, -3.5, 8.75}, {0.125, 7.0, 2.5}, {0.31, -1.27, 2.11});
  const auto product = value * strobe::inverse(value);

  for (uint32_t row = 0; row < 4; ++row) {
    for (uint32_t column = 0; column < 4; ++column) {
      EXPECT_NEAR(product.at(row, column), row == column ? 1.0 : 0.0,
                  1e-12);
    }
  }
}
