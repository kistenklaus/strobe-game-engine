#include "renderer/Source.hpp"

namespace sge {

ISource::ISource(RenderPass& renderPass) : m_renderPass(renderPass) {}
RenderPass& ISource::getRenderPass() { return m_renderPass; }

}  // namespace sge
