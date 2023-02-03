#pragma once

#include "Geometrie.h"
#include "../buffer/BufferManager.h"
#include <glad/glad.h>
#include "../resource_id.h"
#include <vector>

namespace strobe::internal {

    class GeometrieManager {
    public:

        explicit GeometrieManager()
                : m_bufferManager(nullptr) {}

        explicit GeometrieManager(BufferManager *bufferManager)
                : m_bufferManager(bufferManager) {}

        ~GeometrieManager();

        void processSubmissions();

        void endFrame();

        Geometrie foreignCreate(std::vector<GeometrieAttribute> attributes,
                                std::shared_ptr<GeometrieIndices> indices = nullptr);

    private:
        struct geometrie_attribute {
        public:
            unsigned int m_location;
            unsigned int m_size;
            GLenum m_type;
            bool m_normalized;
            unsigned int m_stride;
            unsigned int m_offset;
            std::weak_ptr<Buffer> m_buffer;
        };

        void foreignDestroyCallback(resource_id resourceId);

        void foreignUpdateCallback(resource_id resourceId);

        void processDestroySubmissions();

        void processCreateSubmissions();

        void processUpdateSubmissions();

    private:
        std::vector<resource_id> m_destroyQueue;
        std::vector<resource_id> m_createQueue;
        std::vector<resource_id> m_updateQueue;

        std::vector<resource_id> m_destroyQueue_back;
        std::vector<resource_id> m_createQueue_back;
        std::vector<resource_id> m_updateQueue_back;

        std::vector<unsigned int> m_refCounts;
        std::vector<std::vector<geometrie_attribute>> m_attributes;
        std::vector<std::weak_ptr<GeometrieIndices>> m_indices;

        std::vector<unsigned int> m_internal_tmp_vector;
        std::vector<unsigned int> m_opengl_vao_names;

        BufferManager *m_bufferManager;
    };


}