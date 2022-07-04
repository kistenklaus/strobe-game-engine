#include "renderer/Renderer.hpp"

#include "renderer/vulkan/VulkanRendererBackend.hpp"

using namespace sge;

Renderer::Renderer(RendererBackendAPI backendApi, Window* window)
    : m_backend([&]() {
        std::string application_name = "name_placeholder";
        std::tuple<int, int, int> application_version = {0, 0, 0};
        std::string engine_name = "strobe-game-engine";
        std::tuple<int, int, int> engine_version = {0, 0, 0};
        switch (backendApi) {
          case VULKAN_RENDER_BACKEND:
            return std::make_unique<vulkan::VulkanRendererBackend>(
                application_name, application_version, engine_name,
                engine_version, window);
        }
        throw std::runtime_error("render backend is not supported");
      }()) {}

void Renderer::beginFrame() { m_backend->beginFrame(); }

void Renderer::renderFrame() { m_backend->renderFrame(); }

void Renderer::endFrame() { m_backend->endFrame(); }
