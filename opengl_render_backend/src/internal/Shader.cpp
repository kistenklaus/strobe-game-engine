#include "glad/glad.h"
#include "Shader.h"

Shader::Shader(const std::string &source, Type type, const std::string& path) : m_type(type) {
    m_id = glCreateShader(m_type);
    const char *c_source = source.c_str();
    glShaderSource(m_id, 1, &c_source, nullptr);
    glCompileShader(m_id);
    int success;
    glGetShaderiv(m_id, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(m_id, 512, nullptr, infoLog);
        throw std::runtime_error("[Fatal Error] Failed to compile shaders [" + path + "] \n"
                                 + std::string(infoLog));
    }
}

Shader::~Shader() {
    glDeleteShader(m_id);
}

Shader::Shader(const TextFileAsset &file, Shader::Type type) : Shader(file.content(), type, file.path()){}
