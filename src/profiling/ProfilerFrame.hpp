#pragma once
#include <chrono>

#include "types/inttypes.hpp"

namespace sge {

struct ProfilerFrame {
  using clock = std::chrono::high_resolution_clock;
  using time = std::chrono::system_clock::time_point;
  using duration = std::chrono::duration<u64, std::ratio<1, 1000000000>>;

  ProfilerFrame();
  time m_startOfFrame;
  duration m_frametime;
  boolean m_valid = false;
};

}  // namespace sge
