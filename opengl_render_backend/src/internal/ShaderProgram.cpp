#include "ShaderProgram.h"
#include "glad/glad.h"
#include <stdexcept>
#include <cassert>

inline static void linkShaders(const unsigned int pid, const std::string &vsPath, const std::string &fsPath) {
    int success;
    glLinkProgram(pid);
    glGetProgramiv(pid, GL_LINK_STATUS, &success);
    if (!success) {
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(pid, 512, nullptr, infoLog);
            throw std::runtime_error("[Fatal Error] Failed to Link Shaders {vertex: ["
                                     + vsPath + "], " + " fragment:[" + fsPath + "]}\n"
                                     + std::string(infoLog));
        }
    }
}

ShaderProgram::ShaderProgram(const Shader &vertexShader, const Shader &fragmentShader) {
    m_id = glCreateProgram();
    assert(vertexShader.m_type == Shader::Type::VertexShader);
    assert(fragmentShader.m_type == Shader::Type::FragmentShader);
    glAttachShader(m_id, vertexShader.m_id);
    glAttachShader(m_id, fragmentShader.m_id);
    linkShaders(m_id, vertexShader.m_path, fragmentShader.m_path);
}

ShaderProgram::ShaderProgram(const std::string &vertexPath, const std::string &fragmentPath) :
        ShaderProgram(Shader(vertexPath, Shader::Type::VertexShader),
                      Shader(fragmentPath, Shader::Type::FragmentShader)) {}

ShaderProgram::~ShaderProgram() {
    glDeleteProgram(m_id);
}

