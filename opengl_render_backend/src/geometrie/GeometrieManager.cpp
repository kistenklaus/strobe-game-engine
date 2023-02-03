#include "GeometrieManager.h"
#include <glad/glad.h>
#include <iostream>
#include <stdexcept>

strobe::internal::GeometrieManager::~GeometrieManager(){
    // Documentation states that invalids vao names are ignored.
    glDeleteVertexArrays(static_cast<GLsizei>(m_opengl_vao_names.size()),
                         m_opengl_vao_names.data());
}

void strobe::internal::GeometrieManager::processSubmissions() {
    processDestroySubmissions();
    processCreateSubmissions();
    processUpdateSubmissions();
}

void strobe::internal::GeometrieManager::endFrame() {
    m_destroyQueue.swap(m_destroyQueue_back);
    m_createQueue.swap(m_createQueue_back);
    m_updateQueue.swap(m_updateQueue_back);
}

strobe::Geometrie strobe::internal::GeometrieManager::foreignCreate(
        std::vector<GeometrieAttribute> attributes,
        std::shared_ptr<GeometrieIndices> indices) {
    // check if there already exists a geometrie with the same attributs.
    for (unsigned int i = 0; i < m_indices.size(); ++i) {
        //const std::shared_ptr<GeometrieIndices>& shared_indices = m_indices[i].lock();
        using wt = std::weak_ptr<GeometrieIndices>;
        bool invalidIndices = !m_indices[i].owner_before(wt{}) && !wt{}.owner_before(m_indices[i]);
        if ((
                    (invalidIndices && (indices == nullptr))
                    || ((invalidIndices) == (indices == nullptr))
                    || (*(m_indices[i].lock()) == *indices)
            ) && m_attributes[i].size() == attributes.size()) {
            std::sort(attributes.begin(), attributes.end());
            //compare attribute
            bool equalAttributes = false;
            for (unsigned int j = 0; j < attributes.size(); ++i) {
                bool invalidAttributes = !m_attributes[i][j].m_buffer.owner_before(wt{}) && !wt{}.owner_before
                        (m_attributes[i][j].m_buffer);
                if (
                        (
                                (invalidAttributes && attributes[j].buffer() == nullptr)
                                ||
                                (
                                        (not invalidAttributes)
                                        && (attributes[j].buffer())
                                        && (m_attributes[i][j].m_buffer.lock() == attributes[j].buffer())
                                )
                        )
                        && m_attributes[i][j].m_location == attributes[j].location()
                        && m_attributes[i][j].m_stride == attributes[j].byteStride()
                        && m_attributes[i][j].m_offset == attributes[j].byteOffset()) {
                    equalAttributes = true;
                    break;
                }
            }
            if (equalAttributes) {
                //TODO compare size and normalized properties to be completly percise.
                m_refCounts[i]++;
                return Geometrie(&m_refCounts[i],
                                 [&](resource_id id) { foreignDestroyCallback(id); },
                                 [&](resource_id id) { foreignUpdateCallback(id); },
                                 std::move(attributes),
                                 std::move(indices),
                                 resource_id(i)
                );
            }
        }
    }
    // find next valid resource_id
    resource_id id = null_id;
    for (unsigned int i = 0; i < m_refCounts.size(); ++i) {
        if (m_refCounts[i] == 0) {
            id = resource_id(i);
        }
    }
    if (not id.isValid()) {
        id = resource_id(m_refCounts.size());
        m_refCounts.push_back(0);
        m_attributes.emplace_back();
        m_indices.emplace_back();
        m_opengl_vao_names.emplace_back();
    }
    // initialize control resources.
    m_refCounts[id.asIntegral()] = 1;
    auto &a = m_attributes[id.asIntegral()];
    for (auto &attrib: attributes) {
        a.emplace_back();
        auto &back = a.back();
        back.m_location = attrib.location();
        back.m_buffer = attrib.buffer();
        back.m_offset = attrib.byteOffset();
        back.m_stride = attrib.byteStride();
        switch (attrib.type()) {
            case GeometrieAttribute::Type::Vec4_32f:
                back.m_size = 4;
                back.m_type = GL_FLOAT;
                back.m_normalized = false;
                break;
            case GeometrieAttribute::Type::Vec3_32f:
                back.m_size = 3;
                back.m_type = GL_FLOAT;
                back.m_normalized = false;
                break;
            case GeometrieAttribute::Type::Vec2_32f:
                back.m_size = 2;
                back.m_type = GL_FLOAT;
                back.m_normalized = false;
                break;
            default:
                throw std::runtime_error("[Unimplemented Switch Case]");
        }
    }
    m_indices[id.asIntegral()] = indices;

    m_createQueue.push_back(id);
    m_updateQueue.push_back(id);

    return Geometrie(
            &m_refCounts[id.asIntegral()],
            [&](resource_id id) { foreignDestroyCallback(id); },
            [&](resource_id id) { foreignUpdateCallback(id); },
            attributes,
            indices,
            id);

}

void strobe::internal::GeometrieManager::foreignDestroyCallback(strobe::internal::resource_id resourceId) {
    m_destroyQueue.push_back(resourceId);
    std::erase_if(m_createQueue, [&](auto rid) { return resourceId == rid; });
    std::erase_if(m_updateQueue, [&](auto rid) { return resourceId == rid; });
}

void strobe::internal::GeometrieManager::foreignUpdateCallback(strobe::internal::resource_id resourceId) {
    m_updateQueue.emplace_back(resourceId);
}

void strobe::internal::GeometrieManager::processDestroySubmissions() {
    if (m_destroyQueue_back.empty())return;
    m_internal_tmp_vector.resize(m_destroyQueue_back.size());
    for (unsigned int i = 0; i < m_destroyQueue_back.size(); ++i) {
        const resource_id& id = m_destroyQueue_back[i];
        m_internal_tmp_vector[i] = id.asIntegral();
        m_opengl_vao_names[id.asIntegral()] = 0;
    }
    glad_glDeleteVertexArrays(static_cast<GLsizei>(m_internal_tmp_vector.size()),
                              m_internal_tmp_vector.data());
    m_destroyQueue_back.clear();
}

void strobe::internal::GeometrieManager::processCreateSubmissions() {
    if (m_createQueue_back.empty())return;
    m_internal_tmp_vector.resize(m_createQueue_back.size());
    glCreateVertexArrays(static_cast<GLsizei>(m_createQueue_back.size()),
                         m_internal_tmp_vector.data());
    for (unsigned int i = 0; i < m_createQueue_back.size(); ++i) {
        m_opengl_vao_names[m_createQueue_back[i].asIntegral()]
                = m_internal_tmp_vector[i];
    }
    m_createQueue_back.clear();
}

void strobe::internal::GeometrieManager::processUpdateSubmissions() {
    while (not m_updateQueue_back.empty()) {
        resource_id id = m_updateQueue_back.back();
        m_updateQueue_back.pop_back();
        unsigned int vao = m_opengl_vao_names[id.asIntegral()];
        glBindVertexArray(vao);
        const std::vector<geometrie_attribute> &attributes = m_attributes[id.asIntegral()];
        for (const geometrie_attribute &attribute: attributes) {
            const std::shared_ptr<Buffer> &vbo = attribute.m_buffer.lock();
            // bind buffer. (requires optimization on the buffer side.)
            // i want something like this
            // vbo->bind();
            // this could be implemented with a cashing variable.
            // Buffer->handle(); should return a resource handle. (which is a opengl name).
            // RenderObject can then contain the handle immediatly.
            m_bufferManager->bind(vbo->resourceId());
            glVertexAttribPointer(attribute.m_location,
                                  static_cast<GLint>(attribute.m_size),
                                  attribute.m_type,
                                  attribute.m_normalized,
                                  static_cast<GLsizei>(attribute.m_stride),
                                  reinterpret_cast<void *>(attribute.m_offset));
            glEnableVertexAttribArray(attribute.m_location);
        }

    }
    m_updateQueue_back.clear();
}

