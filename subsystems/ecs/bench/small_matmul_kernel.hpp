#pragma once
#include <cstdint>

template <uint32_t N, typename T = float> struct SmallMatMulKernel {
  static constexpr uint64_t muls_per_matrix() noexcept {
    return uint64_t{N} * N * N;
  }

  static constexpr uint64_t adds_per_matrix() noexcept {
    return uint64_t{N} * N * N;
  }

  static constexpr uint64_t flops_per_matrix() noexcept {
    return 2ull * uint64_t{N} * N * N;
  }

  static constexpr uint64_t bytes_per_matrix_nominal() noexcept {
    // Read A, read B, write C once.
    return 3ull * uint64_t{N} * N * sizeof(T);
  }

  static constexpr double arithmetic_intensity_nominal() noexcept {
    return double(flops_per_matrix()) / double(bytes_per_matrix_nominal());
  }

  static void run(T const *a, T const *b, T *c) noexcept {
    for (uint32_t i = 0; i < N; ++i) {
      for (uint32_t j = 0; j < N; ++j) {
        T acc = T{0};

        for (uint32_t k = 0; k < N; ++k) {
          acc += a[i * N + k] * b[k * N + j];
        }

        c[i * N + j] = acc;
      }
    }
  }
};
