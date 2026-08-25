#pragma once

#if defined(_MSC_VER)
#define STROBE_ALWAYS_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define STROBE_ALWAYS_INLINE inline __attribute__((always_inline))
#else
#define STROBE_ALWAYS_INLINE inline
#endif
