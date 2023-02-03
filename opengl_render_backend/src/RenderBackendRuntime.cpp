#include "RenderBackendRuntime.h"
#include "GlfwWindow.h"
#include <iostream>


strobe::RenderBackendRuntime::RenderBackendRuntime()
        : m_bufferManager(),
          m_geometrieManager(),
          m_requestShutdown(false),
          m_thread() {
}

strobe::RenderBackendRuntime::RenderBackendRuntime(std::shared_ptr<std::binary_semaphore> validRuntimeSignal)
        : m_bufferManager(),
          m_geometrieManager(&m_bufferManager),
          m_requestShutdown(false),
          m_thread(new std::thread(&RenderBackendRuntime::main, this, std::move(validRuntimeSignal))) {
    std::cout << "constructed runtime" << std::endl;
}

strobe::RenderBackendRuntime::~RenderBackendRuntime() {
    std::cout << "destruct runtime" << std::endl;
    requestShutdown();
    if (m_thread != nullptr && m_thread->joinable()) {
        m_thread->join();
    }

    delete m_thread;
}


void strobe::RenderBackendRuntime::startup() {
    GlfwWindow::CreateInfo createInfo = {

    };
    m_window = new GlfwWindow(&createInfo);
}

void strobe::RenderBackendRuntime::shutdown() {
    m_window->close();
    delete m_window;
}

void strobe::RenderBackendRuntime::main(std::shared_ptr<std::binary_semaphore> validRuntimeSignal) {
    std::cout << "started runtime at pid=" << std::this_thread::get_id() << std::endl;
    m_running = true;
    startup();
    validRuntimeSignal->release();
    while (!m_requestShutdown) {
        GlfwWindow::pollEvents();
        if (m_window->shouldClose())m_requestShutdown = true;
        renderloop();
        m_window->swapBuffers();
    }
    m_running = false;
    shutdown();

    m_runtimeStoppedSignal.release();

    std::cout << "runtime stopped execution" << std::endl;
}

void strobe::RenderBackendRuntime::renderloop() {
    // ======== MEMORY CRITICAL SECTION ==========
    m_frameBeginSemaphore.acquire();

    m_bufferManager.processSubmissions();
    m_geometrieManager.processSubmissions();


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //swaps queue buffers (no submissions are allowed during this section).
    m_bufferManager.endFrame();
    m_geometrieManager.endFrame();

    m_frameEndSemaphore.release();
}

void strobe::RenderBackendRuntime::requestShutdown() {
    m_requestShutdown = true;
}

void strobe::RenderBackendRuntime::awaitShutdown() {
    m_runtimeStoppedSignal.acquire();
}

void strobe::RenderBackendRuntime::beginFrame() {
    if (not m_running)return;
    m_frameBeginSemaphore.release();
}

void strobe::RenderBackendRuntime::endFrame() {
    using namespace std::chrono_literals;
    bool succ;
    do {
        succ = m_frameEndSemaphore.try_acquire_for(1ms);
    } while (!succ && m_running);
}


strobe::Buffer strobe::RenderBackendRuntime::createBuffer(
        const std::shared_ptr<ReadSharedMemoryBuffer<char>> &memory,
        Buffer::Usage usage,
        Buffer::Type type) {
    return m_bufferManager.foreignCreate(memory, usage, type);
}

strobe::Geometrie strobe::RenderBackendRuntime::createGeometrie(const std::vector<GeometrieAttribute>& attributes,
                          const std::shared_ptr<GeometrieIndices>& indices){
    return m_geometrieManager.foreignCreate(attributes, indices);
}
