#include "profiling/Profiler.hpp"

#include "logging/print.hpp"

namespace sge {

Profiler::Profiler() {}

void Profiler::beginFrameInternal() {
  m_frames[m_topIndex].m_startOfFrame = clock::now();
}

void Profiler::endFrameInternal() {
  time endOfFrame = clock::now();
  duration frametime = endOfFrame - m_frames[m_topIndex].m_startOfFrame;
  if (m_frames[m_topIndex].m_valid) {
    m_frametimeSum -= m_frames[m_topIndex].m_frametime;
  } else {
    m_frames[m_topIndex].m_valid = true;
    m_frameCount++;
  }
  m_frametimeSum += frametime;
  m_frames[m_topIndex].m_frametime = frametime;
  m_topIndex = (m_topIndex + 1) % m_frames.size();
}

void Profiler::pushInternal(const std::string& sid) {}

void Profiler::popInternal() {}

Profiler::dduration Profiler::getAverageFrametimeInternal() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(m_frametimeSum) /
         (double)m_frameCount;
}

void Profiler::logInfoInternal() {
  if (m_topIndex == 0) {
    print("frametime:");
    print(getAverageFrametimeInternal().count() / 1e6);
    print(" ---> ");
    print((int)(1.0 / (getAverageFrametimeInternal().count() / 1e9)));
    print(" FPS");
    println("ms");
  }
}

}  // namespace sge
