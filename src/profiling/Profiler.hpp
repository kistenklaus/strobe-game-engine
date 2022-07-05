
#include <array>
#include <chrono>
#include <string>
#include <vector>

#include "profiling/ProfilerFrame.hpp"
#include "types/inttypes.hpp"

namespace sge {

class Profiler {
  using clock = std::chrono::high_resolution_clock;
  using time = std::chrono::system_clock::time_point;
  using duration = std::chrono::duration<i64, std::ratio<1, 1000000000>>;
  using dduration = std::chrono::duration<double, std::ratio<1, 1000000000>>;

 private:
  static const u32 PROFILE_FRAME_COUNT;
  static Profiler& getInstance() {
    static Profiler instance = Profiler();
    return instance;
  }

 public:
  static void push(const std::string& sid) { getInstance().pushInternal(sid); }
  static void pop() { getInstance().popInternal(); }
  static void beginFrame() { getInstance().beginFrameInternal(); }
  static void endFrame() { getInstance().endFrameInternal(); }
  static dduration getAverageFrametime() {
    return getInstance().getAverageFrametimeInternal();
  }
  static void logInfo() { getInstance().logInfoInternal(); }

 private:
  Profiler();
  void beginFrameInternal();
  void endFrameInternal();
  void pushInternal(const std::string& sid);
  void popInternal();
  dduration getAverageFrametimeInternal();
  void logInfoInternal();

 private:
  time m_startOfFrame;
  std::array<ProfilerFrame, 1000> m_frames;
  u32 m_topIndex = 0;
  u32 m_bottomIndex = 0;
  duration m_frametimeSum = duration::zero();
  u32 m_frameCount;
};

}  // namespace sge
