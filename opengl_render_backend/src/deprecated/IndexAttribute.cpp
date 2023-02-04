#include "IndexAttribute.h"

IndexAttribute::IndexAttribute(std::shared_ptr<Buffer> buffer, IndexAttribute::DrawMode mode, unsigned int drawCount)
        : m_buffer(std::move(buffer)),
          m_mode(mode),
          m_drawCount(drawCount) {
    assert(m_buffer != nullptr);
    assert(m_mode != INVALID_MODE);
}

unsigned int IndexAttribute::getDrawCount() const {
    return m_drawCount;
}

void IndexAttribute::setDrawCount(unsigned int drawCount) {
    m_drawCount = drawCount;
}

