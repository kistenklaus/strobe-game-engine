#include "renderer/Renderable.hpp"

namespace sge {

void Renderable::addTechnique(std::shared_ptr<Technique> technique) {
  m_techniques.push_back(technique);
}
void Renderable::removeTechnique(std::shared_ptr<Technique> technique) {
  for (uint32_t i = 0; i < m_techniques.size(); i++) {
    if (m_techniques[i].get() == technique.get()) {
      m_techniques.erase(m_techniques.begin() + i);
    }
  }
}

}  // namespace sge
