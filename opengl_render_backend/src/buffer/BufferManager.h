#pragma once

#include "../resource_id.h"
#include <glad/glad.h>
#include <algorithm>
#include <condition_variable>
#include <optional>
#include <mutex>
#include <memory>
#include <strobe/shared_memory_buffer.h>
#include "Buffer.h"
#include <vector>
#include <mutex>

namespace strobe::internal {

    class BufferManager {
    public:

        ~BufferManager();

        void processSubmissions();

        void endFrame();

        Buffer foreignCreate(
                std::shared_ptr<ReadSharedMemoryBuffer<char>> memory,
                Buffer::Usage usage,
                Buffer::Type type);

        void bind(resource_id bufferId) {
            glBindBuffer(BufferTypeMapping[static_cast<int>(m_buffer_types[bufferId.asIntegral()])],
                         m_opengl_buffer_names[bufferId.asIntegral()]);
        }

        void unbind(resource_id bufferId){
            glBindBuffer(BufferTypeMapping[static_cast<int>(m_buffer_types[bufferId.asIntegral()])],
                         0);
        }

    private:
        const GLenum BufferTypeMapping[3] = {
                GL_ARRAY_BUFFER,
                GL_ELEMENT_ARRAY_BUFFER,
                GL_UNIFORM_BUFFER,
        };

        const GLenum BufferUsageMapping[2] = {
                GL_DYNAMIC_DRAW,
                GL_STATIC_DRAW
        };


        void foreignDestroyCallback(resource_id resourceId) {
            m_destroyQueue.push_back(resourceId);
            std::erase_if(m_createQueue, [&](auto rid) { return resourceId == rid; });
            std::erase_if(m_updateQueue, [&](auto job) { return resourceId == std::get<0>(job); });
        }

        void foreignUpdateCallback(resource_id resourceId, unsigned int startByte, unsigned int endByte) {
            for (auto &[id, start, end]: m_updateQueue) {
                if (startByte < start || endByte > end) {
                    start = std::min(startByte, start);
                    end = std::max(endByte, end);
                    return;
                }
            }
            m_updateQueue.emplace_back(resourceId, startByte, endByte);
        }

        void processDestroySubmissions();

        void processCreateSubmissions();

        void processUpdateSubmissions();


    private:
        std::vector<std::weak_ptr<ReadSharedMemoryBuffer<char>>> m_buffer_memories;
        std::vector<Buffer::Usage> m_buffer_usages;
        std::vector<Buffer::Type> m_buffer_types;
        std::vector<unsigned int> m_referenceCounts;

        std::vector<resource_id> m_destroyQueue;
        std::vector<std::tuple<resource_id, unsigned int, unsigned int>> m_updateQueue;
        std::vector<resource_id> m_createQueue;

        std::vector<resource_id> m_destroyQueue_back;
        std::vector<std::tuple<resource_id, unsigned int, unsigned int>> m_updateQueue_back;
        std::vector<resource_id> m_createQueue_back;

        std::vector<unsigned int> m_opengl_buffer_names;
        std::vector<unsigned int> m_internal_tmp_vector;
    };
}
