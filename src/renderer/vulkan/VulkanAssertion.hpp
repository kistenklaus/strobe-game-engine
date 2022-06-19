#pragma once

#include <signal.h>

#define ASSERT_VKRESULT(result) \
  if (result != VK_SUCCESS) {   \
    raise(SIGTRAP);             \
  }
