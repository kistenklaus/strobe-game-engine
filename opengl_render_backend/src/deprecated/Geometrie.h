#pragma once

#include <utility>
#include <vector>
#include <memory>
#include "VertexAttribute.h"
#include "IndexAttribute.h"
#include <cassert>
#include <algorithm>
#include "glad/glad.h"
#include <optional>



class Geometrie {
public:
    explicit Geometrie(std::vector<VertexAttribute> attributes,
                       std::optional<IndexAttribute> indices);

    ~Geometrie();

    void bind() const;

    static void unbind();


    void drawElements(bool bindBefore = false, bool unbindAfter = false);


private:
    std::vector<VertexAttribute> m_attributes;
    std::optional<IndexAttribute> m_indices;
    unsigned int m_vaoId;
    GLenum m_drawMode;
};