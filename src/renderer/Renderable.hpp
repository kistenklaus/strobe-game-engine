#pragma once

#include <vector>

#include "renderer/Technique.hpp"

namespace sge {

class Renderable {
 public:
  void addTechnique(std::shared_ptr<Technique> technique);
  void removeTechnique(std::shared_ptr<Technique> technique);
  // Transform
 private:
  std::vector<std::shared_ptr<Technique>> m_techniques;
};

}  // namespace sge
