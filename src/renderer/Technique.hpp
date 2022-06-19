#pragma once

#include <cassert>
#include <memory>
#include <vector>

#include "renderer/Step.hpp"
#include "types/inttypes.hpp"

namespace sge {

class Technique {
 private:
  boolean m_enabled;
  std::vector<std::shared_ptr<Step>> m_steps;

 public:
  void addStep(std::shared_ptr<Step> step);
  boolean removeStep(std::shared_ptr<Step> step);
  inline void enable() { m_enabled = true; }
  inline void disable() { m_enabled = false; }
  // for each implementation!!!
};

void Technique::addStep(std::shared_ptr<Step> step) {
  assert([&]() {  // raise if step is contained in m_steps
    for (const auto& s : m_steps) {
      if (s == step) return false;
    }
    return true;
  }());
  m_steps.push_back(step);
}

boolean Technique::removeStep(std::shared_ptr<Step> step) {
  for (u64 i = 0; i < m_steps.size(); i++) {
    if (m_steps[i] == step) {
      if (m_steps.size() == 1) {
        m_steps.clear();
        return true;
      } else {
        m_steps[i] = m_steps[m_steps.size() - 1];
        m_steps.resize(m_steps.size() - 1);
        return true;
      }
    }
  }
  return false;
}

}  // namespace sge
