#pragma once

#include <strobe/assets.h>
#include <glad/glad.h>
#include "ShaderProgram.h"
#include <deque>
#include <utility>
#include <vector>
#include <memory>
#include "../resource_id.h"

namespace strobe::internal {


    class ShaderProgramManager {
    private:

        struct ShaderSources {
            explicit ShaderSources(std::string vertexSrc,
                                   std::string fragmentSrc)
                    : m_vertexSource(std::move(vertexSrc)),
                      m_fragmentSource(std::move(fragmentSrc)) {}

            std::string m_vertexSource;
            std::string m_fragmentSource;
        };

        struct ShaderProgramControlBlock {
            std::string m_vertexPath;
            std::string m_fragmentPath;
            std::unique_ptr<ShaderSources> m_sources;
        };
    public:
        explicit ShaderProgramManager() = default;

        ShaderProgram foreignCreate(const std::shared_ptr<ShaderAsset>& asset) {
            // check if the program already exists.
            for (unsigned int i = 0; i < m_programControlBlocks.size(); i++) {
                if (asset->vertexPath() == m_programControlBlocks[i].m_vertexPath
                    && asset->fragmentPath() == m_programControlBlocks[i].m_fragmentPath) {
                    m_refCounts[i]++;
                    return ShaderProgram(
                            &m_refCounts[i],
                            [&](resource_id id) { foreignDestroyCallback(id); },
                            resource_id(i)
                    );
                }
            }
            // find new resource id.
            resource_id id = null_id;
            unsigned int j = 0;
            for (unsigned int refCount: m_refCounts) {
                if (refCount == 0) {
                    id = resource_id(j);
                    break;
                }
            }
            if (not id.isValid()) {
                m_refCounts.push_back(0);
                m_programControlBlocks.emplace_back();
                m_opengl_program_names.emplace_back();
            }
            unsigned int int_id = id.asIntegral();
            m_refCounts[int_id]++;
            m_programControlBlocks[int_id].m_vertexPath = asset->vertexPath();
            m_programControlBlocks[int_id].m_fragmentPath = asset->fragmentPath();
            m_programControlBlocks[int_id].m_sources =
                    std::make_unique<ShaderSources>(asset->vertexSource(), asset->fragmentSource());
            m_opengl_program_names[int_id] = 0;
            return ShaderProgram(
                    &m_refCounts[int_id],
                    [&](resource_id id) { foreignDestroyCallback(id); },
                    id);
        }

        void processSubmissions() {
            processDestroySubmissions();
            processCreateSubmissions();
        }

        void swapBuffers() {
            m_destroyQueue.swap(m_destroyQueue_back);
            m_createQueue.swap(m_createQueue_back);
        }

    private:

        void foreignDestroyCallback(resource_id id) {
            m_destroyQueue.push_back(id);
            std::erase_if(m_createQueue,
                          [&](auto rid) { return id == rid; });
        }

        void processDestroySubmissions() {
            while (not m_destroyQueue_back.empty()) {
                resource_id id = m_destroyQueue_back.back();
                unsigned int glName = m_opengl_program_names[id.asIntegral()];
                assert(glName != 0);
                glDeleteProgram(glName);
                m_opengl_program_names[id.asIntegral()] = 0;

                m_destroyQueue_back.pop_back();
            }
        }

        static unsigned int compileShaderHelper(GLenum shaderType,
                                                const std::string &source,
                                                const std::string &path) {
            unsigned int name = glCreateShader(shaderType);
            const char *c_source = source.c_str();
            glShaderSource(name, 1, &c_source, nullptr);
            glCompileShader(name);
            int success;
            glGetShaderiv(name, GL_COMPILE_STATUS, &success);
            if (!success) {
                char infoLog[512];
                glGetShaderInfoLog(name, 512, nullptr, infoLog);
                throw std::runtime_error("[Fatal Error] Failed to compile shaders [" + path + "] \n"
                                         + std::string(infoLog));
            }
            return name;
        }

        void processCreateSubmissions() {
            while (not m_createQueue_back.empty()) {
                resource_id id = m_createQueue_back.back();
                unsigned int idi = id.asIntegral();

                std::unique_ptr<ShaderSources> sources = std::move(m_programControlBlocks[idi].m_sources);

                const std::string& vsPath = m_programControlBlocks[idi].m_vertexPath;
                const std::string& fsPath = m_programControlBlocks[idi].m_fragmentPath;

                unsigned int vertexName = compileShaderHelper(
                        GL_VERTEX_SHADER,
                        sources->m_vertexSource,
                        vsPath);
                unsigned int fragmentName = compileShaderHelper(
                        GL_FRAGMENT_SHADER,
                        sources->m_fragmentSource,
                        fsPath);
                unsigned int glName = glCreateProgram();
                glAttachShader(glName, vertexName);
                glAttachShader(glName, fragmentName);
                glLinkProgram(glName);

                GLint success;
                glGetProgramiv(glName, GL_LINK_STATUS, &success);
                if (!success) {
                    if (!success) {
                        char infoLog[512];
                        glGetProgramInfoLog(glName, 512, nullptr, infoLog);
                        std::string err = "[Fatal Error] Failed to Link Shaders";
                        err.append("{vertex: [");
                        err.append(vsPath);
                        err.append("] fragment: [");
                        err.append("]}\n");
                        err.append(infoLog);
                        throw std::runtime_error(err);
                    }
                }
                m_opengl_program_names[idi] = glName;

                m_createQueue_back.pop_back();
            }
        }

    private:
        std::vector<resource_id> m_destroyQueue;
        std::vector<resource_id> m_createQueue;

        std::vector<resource_id> m_destroyQueue_back;
        std::vector<resource_id> m_createQueue_back;

        std::vector<ShaderProgramControlBlock> m_programControlBlocks; //shader_program_control_blocks
        std::vector<unsigned int> m_opengl_program_names;

        std::deque<unsigned int> m_refCounts;
    };

}