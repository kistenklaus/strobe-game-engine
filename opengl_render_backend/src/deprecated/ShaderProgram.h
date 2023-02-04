#pragma once

#include "Shader.h"
#include "glad/glad.h"
#include "glm/ext/matrix_float4x4.hpp"
#include <memory>

class ShaderProgram {
    friend class Uniform;

public:
    explicit ShaderProgram(const Shader &vertexShader,
                           const Shader &fragmentShader);

    explicit ShaderProgram(const std::string &vertexPath,
                           const std::string &fragmentPath);

    ~ShaderProgram();

    inline void bind() const { glUseProgram(m_id); }

    [[maybe_unused]] static inline void unbind() { glUseProgram(0); }


    inline void uniform1f(const std::string &name, float value) const {
        glUniform1f(fetchUniformLocationByName(name), value);
    }

    inline void uniform1i(const std::string &name, int32_t value) const {
        glUniform1i(fetchUniformLocationByName(name), value);
    }

    inline void uniform1ui(const std::string &name, uint32_t value) const {
        glUniform1ui(fetchUniformLocationByName(name), value);
    }

    inline void uniform2f(const std::string &name, float value1, float value2) const {
        glUniform2f(fetchUniformLocationByName(name), value1, value2);
    }

    inline void uniform2i(const std::string &name, int32_t value1, int32_t value2) const {
        glUniform2i(fetchUniformLocationByName(name), value1, value2);
    }

    inline void uniform2ui(const std::string &name, uint32_t value1, uint32_t value2) const {
        glUniform2ui(fetchUniformLocationByName(name), value1, value2);
    }

    inline void uniform3f(const std::string &name, float value1, float value2, float value3) const {
        glUniform3f(fetchUniformLocationByName(name), value1, value2, value3);
    }

    inline void uniformVec3(const std::string &name, glm::vec3& value) const {
        glUniform3fv(fetchUniformLocationByName(name), 1, reinterpret_cast<const GLfloat *>(&value));
    }


    inline void uniform3i(const std::string &name, int32_t value1, int32_t value2, int32_t value3) const {
        glUniform3i(fetchUniformLocationByName(name), value1, value2, value3);
    }

    inline void uniform3ui(const std::string &name, uint32_t value1, uint32_t value2, uint32_t value3) const {
        glUniform3ui(fetchUniformLocationByName(name), value1, value2, value3);
    }

    inline void uniform4f(const std::string &name, float value1, float value2, float value3, float value4) const {
        glUniform4f(fetchUniformLocationByName(name), value1, value2, value3, value4);
    }

    inline void uniform4i(const std::string &name, int32_t value1, int32_t value2, int32_t value3, int32_t value4)
    const {
        glUniform4i(fetchUniformLocationByName(name), value1, value2, value3, value4);
    }

    inline void uniform4ui(const std::string &name, uint32_t value1, uint32_t value2, uint32_t value3, uint32_t value4)
    const {
        glUniform4ui(fetchUniformLocationByName(name), value1, value2, value3, value4);
    }

    inline void uniform1fv(const std::string &name, int count, float *ptr) const {
        glUniform1fv(fetchUniformLocationByName(name), count, ptr);
    }

    inline void uniform1iv(const std::string &name, int count, int32_t *ptr) const {
        glUniform1iv(fetchUniformLocationByName(name), count, ptr);
    }

    inline void uniform1uiv(const std::string &name, int count, uint32_t *ptr) const {
        glUniform1uiv(fetchUniformLocationByName(name), count, ptr);
    }

    inline void uniform2fv(const std::string &name, int count, float *ptr) const {
        glUniform2fv(fetchUniformLocationByName(name), count, ptr);
    }

    inline void uniform2iv(const std::string &name, int count, int32_t *ptr) const {
        glUniform2iv(fetchUniformLocationByName(name), count, ptr);
    }

    inline void uniform2uiv(const std::string &name, int count, uint32_t *ptr) const {
        glUniform2uiv(fetchUniformLocationByName(name), count, ptr);
    }

    inline void uniform3fv(const std::string &name, int count, float *ptr) const {
        glUniform3fv(fetchUniformLocationByName(name), count, ptr);
    }

    inline void uniform3iv(const std::string &name, int count, int32_t *ptr) const {
        glUniform3iv(fetchUniformLocationByName(name), count, ptr);
    }

    inline void uniform3uiv(const std::string &name, int count, uint32_t *ptr) const {
        glUniform3uiv(fetchUniformLocationByName(name), count, ptr);
    }

    inline void uniform4fv(const std::string &name, int count, float *ptr) const {
        glUniform4fv(fetchUniformLocationByName(name), count, ptr);
    }

    inline void uniform4iv(const std::string &name, int count, int32_t *ptr) const {
        glUniform4iv(fetchUniformLocationByName(name), count, ptr);
    }

    inline void uniform4uiv(const std::string &name, int count, uint32_t *ptr) const {
        glUniform4uiv(fetchUniformLocationByName(name), count, ptr);
    }

    inline void uniformMatrix4fv(const std::string &name, const glm::mat4& value) const{
        glUniformMatrix4fv(fetchUniformLocationByName(name),
                           1, false, reinterpret_cast<const GLfloat *>(&value));
    }

    [[nodiscard]] inline unsigned int getUniformBlockIndex(const std::string& name) const{
        return glGetUniformBlockIndex(m_id, name.c_str());
    }

private:

    [[nodiscard]] int fetchUniformLocationByName(const std::string &name) const {
        int location = glGetUniformLocation(m_id, name.c_str());
        if (location == -1) {
            throw std::invalid_argument("[Fatal Error] The uniform " + name
                                        + " has a invalid type or doesn't exist");
        }
        return location;
    }

    unsigned int m_id;
};
