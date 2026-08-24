#pragma once

#define STROBE_DETAIL_PRAGMA_IMPL(...) _Pragma(#__VA_ARGS__)
#define STROBE_DETAIL_PRAGMA(...) STROBE_DETAIL_PRAGMA_IMPL(__VA_ARGS__)

#if defined(__clang__)
#define STROBE_UNROLL(LENGTH)                                                  \
  STROBE_DETAIL_PRAGMA(clang loop unroll_count(LENGTH))
#elif defined(__GNUC__)
#define STROBE_UNROLL(LENGTH) STROBE_DETAIL_PRAGMA(GCC unroll LENGTH)
#else
#define STROBE_UNROLL(LENGTH)
#endif
