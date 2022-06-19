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
  void enable();
  void disable();
  // for each implementation!!!
};

}  // namespace sge
