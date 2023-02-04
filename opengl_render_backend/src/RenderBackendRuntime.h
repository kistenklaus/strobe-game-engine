#pragma once

#include <strobe/shared_memory_buffer.h>
#include "renderobj/RenderObjectQueue.h"
#include "renderobj/RenderObject.h"
#include "./geometrie/GeometrieManager.h"
#include <memory>
#include <glm/glm.hpp>
#include <semaphore>
#include <thread>
#include "GlfwWindow.h"
#include "./buffer/BufferManager.h"
//
#include "deprecated/ShaderProgram.h"
#include "deprecated/Shader.h"

namespace strobe::internal {

    class RenderBackendRuntime {
    public:
        RenderBackendRuntime();

        explicit RenderBackendRuntime(std::shared_ptr<std::binary_semaphore> validRuntimeSignal);


        RenderBackendRuntime(RenderBackendRuntime &) = delete;

        RenderBackendRuntime &operator=(RenderBackendRuntime &) = delete;

        ~RenderBackendRuntime();

        void requestShutdown();

        void awaitShutdown();

        [[nodiscard]] bool isRunning() {
            return m_running;
        }

        Buffer createBuffer(const std::shared_ptr<ReadSharedMemoryBuffer<char>>& memory,
                            Buffer::Usage usage,
                            Buffer::Type type);

        Geometrie createGeometrie(const std::vector<GeometrieAttribute>& attributes,
                                  const std::shared_ptr<GeometrieIndices>& indices = nullptr);

        void beginFrame();

        void endFrame();

        void draw(const RenderObject& renderObject);

    private:

        void startup();

        void main(std::shared_ptr<std::binary_semaphore> validRuntimeSignal);

        void renderloop();

        void shutdown();

        void runtimeBeginFrame();

        void runtimeEndFrame();

        void runtimeCompleteFrame();

    private:

        internal::BufferManager m_bufferManager;
        internal::GeometrieManager m_geometrieManager;

        std::atomic<bool> m_running = false;
        GlfwWindow *m_window = nullptr;
        bool m_requestShutdown;
        std::thread *m_thread;
        std::binary_semaphore m_runtimeStoppedSignal{0};

        std::binary_semaphore m_beginFrameSemaphore{0};
        std::binary_semaphore m_endFrameSemaphore{0};
        std::binary_semaphore m_completeFrameSemaphore{0};

        std::mutex m_frameBeginMutex;
        std::condition_variable m_frameBeginCV;
        bool m_frameReady = false;

        std::mutex m_frameEndMutex;
        std::condition_variable m_frameEndCV;
        bool m_frameFinished = false;

        RenderObjectQueue m_renderObjectQueue;

        std::unique_ptr<ShaderProgram> m_deprecatedShader;

    };



}