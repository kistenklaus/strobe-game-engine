#pragma once
#include <signal.h>

#include "strb/apis/vulkan/vulkan.lib.hpp"
#include "strb/stl.hpp"
#define ASSERT_VKRESULT_WARNINGS

#ifndef NDEBUG

#ifdef ASSERT_VKRESULT_WARNINGS

#define ASSERT_VKRESULT(result)              \
  if (result == VK_SUBOPTIMAL_KHR) {         \
    strb::logln("WARNING: SUB_OPTIMAL_KHR"); \
  } else if (result != VK_SUCCESS) {         \
    raise(SIGTRAP);                          \
  }

#else

#define ASSERT_VKRESULT(result)                              \
  if (result != VK_SUBOPTIMAL_KHR && result != VK_SUCCESS) { \
    raise(SIGTRAP);                                          \
  }

#endif

#else

#define ASSERT_VKRESULT(result)

#endif
