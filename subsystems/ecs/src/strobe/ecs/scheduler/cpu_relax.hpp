#pragma once

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) ||               \
    defined(_M_IX86)
#include <immintrin.h>
#else
#include <atomic>
#endif

namespace strobe::ecs::details {
[[maybe_unused]] static void cpu_relax() noexcept {
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) ||               \
    defined(_M_IX86)
  _mm_pause();
#else
  std::atomic_signal_fence(std::memory_order_seq_cst);
#endif
}
} // namespace strobe::ecs::details
