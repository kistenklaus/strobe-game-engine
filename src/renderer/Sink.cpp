#include "renderer/Sink.hpp"

namespace sge {

ISink::ISink(RenderPass& pass) : m_pass(pass) {}
RenderPass& ISink::getPass() { return m_pass; }
void ISink::link(ISource& source) { m_source = &source; }
}  // namespace sge
