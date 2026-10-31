#include <strobe/core/lina/mat.hpp>
#include <strobe/core/lina/quaternion.hpp>

#include <benchmark/benchmark.h>

namespace {

strobe::mat4 matrix_a = strobe::mat4::transform(
    {12.5F, -7.25F, 3.0F}, {1.25F, 0.75F, 2.0F},
    {0.31F, -1.17F, 2.09F});
strobe::mat4 matrix_b = strobe::mat4::transform(
    {-2.0F, 8.5F, 14.25F}, {0.5F, 3.0F, 1.5F},
    {-0.73F, 0.22F, 1.41F});
strobe::vec4 vector4(1.25F, -3.5F, 8.75F, 1.0F);
strobe::vec3 vector3(1.25F, -3.5F, 8.75F);
strobe::vec3 position(12.5F, -7.25F, 3.0F);
strobe::vec3 scale(1.25F, 0.75F, 2.0F);
strobe::vec3 angles(0.31F, -1.17F, 2.09F);
strobe::quaternion rotation =
    strobe::quaternion::from_euler(angles);
strobe::quaternion interpolation_target =
    strobe::quaternion::from_euler({-0.73F, 0.22F, 1.41F});

void mat4_multiply(benchmark::State &state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(matrix_a);
    benchmark::DoNotOptimize(matrix_b);
    auto result = matrix_a * matrix_b;
    benchmark::DoNotOptimize(result);
  }
}

void mat4_vector_multiply(benchmark::State &state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(matrix_a);
    benchmark::DoNotOptimize(vector4);
    auto result = matrix_a * vector4;
    benchmark::DoNotOptimize(result);
  }
}

void affine_composition(benchmark::State &state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(matrix_a);
    benchmark::DoNotOptimize(matrix_b);
    auto result = matrix_a * matrix_b;
    benchmark::DoNotOptimize(result);
  }
}

void quaternion_vector_rotation(benchmark::State &state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(rotation);
    benchmark::DoNotOptimize(vector3);
    auto result = rotation * vector3;
    benchmark::DoNotOptimize(result);
  }
}

void transform_construction_from_euler(benchmark::State &state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(position);
    benchmark::DoNotOptimize(scale);
    benchmark::DoNotOptimize(angles);
    auto result = strobe::mat4::transform(position, scale, angles);
    benchmark::DoNotOptimize(result);
  }
}

void transform_construction_from_quaternion(benchmark::State &state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(position);
    benchmark::DoNotOptimize(scale);
    benchmark::DoNotOptimize(rotation);
    auto result = strobe::mat4::transform(position, scale, rotation);
    benchmark::DoNotOptimize(result);
  }
}

void quaternion_slerp(benchmark::State &state) {
  constexpr float interpolation = 0.37F;
  for (auto _ : state) {
    benchmark::DoNotOptimize(rotation);
    benchmark::DoNotOptimize(interpolation_target);
    auto result =
        strobe::slerp(rotation, interpolation_target, interpolation);
    benchmark::DoNotOptimize(result);
  }
}

BENCHMARK(mat4_multiply);
BENCHMARK(mat4_vector_multiply);
BENCHMARK(affine_composition);
BENCHMARK(quaternion_vector_rotation);
BENCHMARK(transform_construction_from_euler);
BENCHMARK(transform_construction_from_quaternion);
BENCHMARK(quaternion_slerp);

} // namespace

BENCHMARK_MAIN();
