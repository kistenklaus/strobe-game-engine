#pragma once

#include "Geometrie.h"
#include <deque>
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
                : m_bufferManager(bufferManager) {
            //m_refCounts.reserve(1000);
        }

        ~GeometrieManager();

        void processSubmissions();

        void swapBuffers();

        Geometrie foreignCreate(std::vector<GeometrieAttribute> attributes,
                                std::shared_ptr<GeometrieIndices> indices = nullptr);


        void bind(resource_id id) {
            assert(m_opengl_vao_names[id.asIntegral()] != 0);
            static unsigned int currentlyBound = 0;
            if(m_opengl_vao_names[id.asIntegral()] == currentlyBound)return;
            currentlyBound = m_opengl_vao_names[id.asIntegral()];
            glBindVertexArray(m_opengl_vao_names[id.asIntegral()]);
        }

        void unbind() { //NOLINT
            glBindVertexArray(0);
        }

        void renderGeometrie(resource_id id) {
            assert(m_opengl_vao_names[id.asIntegral()] != 0);
            bind(id);
            //std::cout << "iwh" << std::endl;
            using wt = std::weak_ptr<GeometrieIndices>;
            bool hasIndices = !(m_indices[id.asIntegral()]
                    .owner_before(wt{}) && !wt{}
                    .owner_before(m_indices[id.asIntegral()]));

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
            if (hasIndices){
                //TODO implement proper drawCounts and index types.
            }else{
                assert(false);
                glDrawArrays(GL_TRIANGLES, 0, 12);
            }

        }

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

        std::deque<unsigned int> m_refCounts;
        std::vector<std::vector<geometrie_attribute>> m_attributes;
        std::vector<std::weak_ptr<GeometrieIndices>> m_indices;

        std::vector<unsigned int> m_internal_tmp_vector;
        std::vector<unsigned int> m_opengl_vao_names;

        BufferManager *m_bufferManager;
    };


}