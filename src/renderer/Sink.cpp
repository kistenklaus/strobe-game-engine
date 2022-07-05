#include "renderer/Sink.hpp"

namespace sge {

ISink::ISink(RenderPass& pass, const boolean nullable)
    : m_pass(pass), m_nullable(nullable) {}
RenderPass& ISink::getPass() { return m_pass; }
void ISink::link(ISource* p_source) { mp_source = p_source; }
}  // namespace sge
