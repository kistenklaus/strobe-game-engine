#pragma once

#include <memory>
#include <utility>
#include <cassert>

class Buffer; //fwd declaration.

class IndexAttribute {
    friend class Geometrie;

public:

    enum DrawMode {
        TRIANGLES,
        INVALID_MODE
    };

    explicit IndexAttribute(std::shared_ptr<Buffer> buffer,
                            DrawMode mode,
                            unsigned int drawCount);

    ~IndexAttribute() = default;


    void setDrawCount(unsigned int drawCount);

    [[nodiscard]] unsigned int getDrawCount() const;

private:
    std::shared_ptr<Buffer> m_buffer;
    DrawMode m_mode;
    unsigned int m_drawCount;
};
