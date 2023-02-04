#pragma once

#include <strobe/double_buffered_memory.h>
#include "RenderObject.h"
#include "../geometrie/Geometrie.h"

namespace strobe::internal {

    class RenderObjectQueue {
    public:

        explicit RenderObjectQueue(unsigned int maxRenderObjects)
                : m_data(maxRenderObjects) {}

        void enqueue(const RenderObject& renderObject) {
            assert(m_enqueueSize < m_data.size());
            RenderObject *ptr = m_data.front() + m_enqueueSize;
            *ptr = renderObject;
            m_enqueueSize++;
        }

        RenderObject *getEnqueuedRenderObjects() {
            return m_data.back();
        }

        [[nodiscard]] unsigned int getEnqueuedObjectCount() const{
            return m_dequeueSize;
        }

        void swapBuffers() {
            m_data.swapBuffers();
            m_dequeueSize = m_enqueueSize;
            m_enqueueSize = 0;
        }

        [[nodiscard]] unsigned int capacity() const {
            return m_data.size();
        }

    private:
        DoubleBufferedMemory<RenderObject> m_data;
        unsigned int m_enqueueSize = 0;
        unsigned int m_dequeueSize = 0;
    };

}