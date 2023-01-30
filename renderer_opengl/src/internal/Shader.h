#pragma once

#include <string>
#include <stdexcept>
#include <cstdio>
#include <cinttypes>
#include <strobe/assets.h>
#include <glad/glad.h>

class Shader {
    friend class ShaderProgram;

public:

    enum Type {
        VertexShader = GL_VERTEX_SHADER,
        FragmentShader = GL_FRAGMENT_SHADER,
    };

    explicit Shader(const std::string &source,
                    Type type,
                    const std::string &path = "no-path-available");

    explicit Shader(const TextFileAsset &file, Type type);

    ~Shader();

private:
    unsigned int m_id;
    const std::string m_path; //is only stored for better error messages!
    const Type m_type;
};
