#include "renderer/Renderer.hpp"

#include "renderer/vulkan/VRendererBackend.hpp"

using namespace sge;

Renderer::Renderer(RendererBackendAPI backendApi, Window* window)
    : m_backend([&]() {
        std::string applicationName = "name_placeholder";
        std::tuple<u32, u32, u32> applicationVersion = {0, 0, 0};
        std::string engineName = "strobe-game-engine";
        std::tuple<u32, u32, u32> engineVersion = {0, 0, 0};
        switch (backendApi) {
          case VULKAN_RENDER_BACKEND:
            return std::make_unique<vulkan::VRendererBackend>(
                applicationName, applicationVersion, engineName, engineVersion,
                window);
        }
        throw std::runtime_error("render backend is not supported");
      }()) {}

void Renderer::beginFrame() { m_backend->beginFrame(); }

void Renderer::renderFrame() { m_backend->renderFrame(); }

void Renderer::endFrame() { m_backend->endFrame(); }
