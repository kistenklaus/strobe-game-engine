#include "BufferManager.h"
#include <iostream>


strobe::internal::BufferManager::~BufferManager() {
    // Documentation states that invalids buffer names are ignored.
    glDeleteBuffers(static_cast<GLsizei>(m_opengl_buffer_names.size()),
                    m_opengl_buffer_names.data());
}

void strobe::internal::BufferManager::processSubmissions() {
    processDestroySubmissions();
    processCreateSubmissions();
    processUpdateSubmissions();
}

void strobe::internal::BufferManager::endFrame() {
    m_destroyQueue.swap(m_destroyQueue_back);
    m_createQueue.swap(m_createQueue_back);
    m_updateQueue.swap(m_updateQueue_back);
}

strobe::Buffer strobe::internal::BufferManager::foreignCreate(
        std::shared_ptr<ReadSharedMemoryBuffer<char>> memory,
        Buffer::Usage usage,
        Buffer::Type type) {
    // check if the resource already exists.
    assert(m_buffer_memories.size() == m_buffer_usages.size());
    assert(m_buffer_memories.size() == m_referenceCounts.size());
    for (unsigned int i = 0; i < m_buffer_memories.size(); ++i) {
        if (m_buffer_memories[i].lock().get() == memory.get()
            && m_buffer_usages[i] == usage
            && m_buffer_types[i] == type) {
            // increment reference counter here.
            m_referenceCounts[i]++;
            return Buffer(&m_referenceCounts[i],
                          [&](resource_id id) { foreignDestroyCallback(id); },
                          [&](resource_id id, unsigned int start, unsigned end) {
                              foreignUpdateCallback(id, start, end);
                          },
                          std::move(memory),
                          usage,
                          type,
                          resource_id(i));
        }
    }
    // find next resource_id ensure that resource_ids are packed.
    resource_id id = null_id;
    for (unsigned int i = 0; i < m_referenceCounts.size(); ++i) {
        if (m_referenceCounts[i]) {
            id = resource_id(i);
        }
    }
    if (not id.isValid()) {
        id = resource_id(m_referenceCounts.size());
        m_referenceCounts.push_back(0);
        m_buffer_memories.emplace_back();
        m_buffer_usages.emplace_back();
        m_buffer_types.emplace_back();
        m_opengl_buffer_names.emplace_back(0);
    }
    auto integral_id = static_cast<resource_id_type>(id);
    m_buffer_memories[integral_id] = memory;
    m_buffer_usages[integral_id] = usage;
    m_buffer_types[integral_id] = type;
    m_referenceCounts[integral_id] = 1;
    // create new buffer (handle).
    // we create two buffers for reference counting.
    m_createQueue.push_back(id);
    m_updateQueue.emplace_back(id, 0, m_buffer_memories[integral_id].lock()->capacity());
    return Buffer(&m_referenceCounts[integral_id],
                  [&](resource_id id) { foreignDestroyCallback(id); },
                  [&](resource_id id, unsigned int start, unsigned end) {
                      foreignUpdateCallback(id, start, end);
                  },
                  std::move(memory),
                  usage,
                  type,
                  id);
}

void strobe::internal::BufferManager::processDestroySubmissions() {
    m_internal_tmp_vector.clear();
    while (not m_destroyQueue_back.empty()) {
        resource_id id = m_destroyQueue_back.back();
        m_destroyQueue_back.pop_back();
        m_internal_tmp_vector.push_back(m_opengl_buffer_names[static_cast<resource_id_type>(id)]);
    }
    glDeleteBuffers(static_cast<GLsizei>(m_internal_tmp_vector.size()),
                    m_internal_tmp_vector.data());
}

void strobe::internal::BufferManager::processCreateSubmissions() {
    if (m_createQueue_back.empty())return;
    m_internal_tmp_vector.resize(m_createQueue_back.size());
    glGenBuffers(static_cast<GLsizei>(m_internal_tmp_vector.size()),
                 m_internal_tmp_vector.data());
    for (unsigned int i = 0; i < m_createQueue_back.size(); ++i) {
        m_opengl_buffer_names[static_cast<resource_id_type>(m_createQueue_back[i])]
                = m_internal_tmp_vector[i];
    }
    m_createQueue_back.clear();
    m_internal_tmp_vector.clear();
}

void strobe::internal::BufferManager::processUpdateSubmissions() {
    while (not m_updateQueue_back.empty()) {
        auto &[id, startByte, endByte] = m_updateQueue_back.back();
        auto integralId = static_cast<resource_id_type>(id);
        m_updateQueue_back.pop_back();
        unsigned int byteRange = endByte - startByte;
        auto memory = m_buffer_memories[integralId].lock();
        memory->acquireNewRead();
        GLenum type = BufferTypeMapping[static_cast<int>(m_buffer_types[integralId])];
        // TODO performance tune this heuristic.
        if (memory->capacity() < byteRange * 2) {
            // reallocate the hole buffer.
            GLenum usage = BufferUsageMapping[static_cast<int>(m_buffer_usages[integralId])];
            glBindBuffer(type, m_opengl_buffer_names[integralId]);
            glBufferData(type,
                         memory->capacity(),
                         memory->data(),
                         usage);
        } else {
            glBufferSubData(type,
                            static_cast<GLintptr>(startByte),
                            static_cast<GLsizeiptr>(endByte),
                            reinterpret_cast<const char *>(memory->data()) + startByte
            );
        }
        memory->releaseRead();
    }
}

