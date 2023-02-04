#include "RenderBackendRuntime.h"
#include "GlfwWindow.h"
#include <chrono>
#include <iostream>

static const unsigned int MAX_RENDER_OBJECTS = 10000;

strobe::internal::RenderBackendRuntime::RenderBackendRuntime()
        : m_bufferManager(),
          m_geometrieManager(),
          m_requestShutdown(false),
          m_thread(),
          m_renderObjectQueue(0) {
}

strobe::internal::RenderBackendRuntime::RenderBackendRuntime(std::shared_ptr<std::binary_semaphore> validRuntimeSignal)
        : m_bufferManager(),
          m_geometrieManager(&m_bufferManager),
          m_requestShutdown(false),
          m_thread(new std::thread(&RenderBackendRuntime::main, this, std::move(validRuntimeSignal))),
          m_renderObjectQueue(MAX_RENDER_OBJECTS) {
    std::cout << "constructed runtime" << std::endl;
}

strobe::internal::RenderBackendRuntime::~RenderBackendRuntime() {
    std::cout << "destruct runtime" << std::endl;
    requestShutdown();
    if (m_thread != nullptr && m_thread->joinable()) {
        m_thread->join();
    }

    delete m_thread;
}


void strobe::internal::RenderBackendRuntime::startup() {
    GlfwWindow::CreateInfo createInfo = {

    };
    m_window = new GlfwWindow(&createInfo);


    TextFileAsset vertexShaderAsset("res/simple.vs.glsl");
    Shader vertexShader = Shader(vertexShaderAsset, Shader::VertexShader);
    TextFileAsset fragmentShaderAsset("res/simple.fs.glsl");
    Shader fragmentShader = Shader(fragmentShaderAsset, Shader::FragmentShader);

    m_deprecatedShader = std::make_unique<ShaderProgram>(vertexShader, fragmentShader);

    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
}

void strobe::internal::RenderBackendRuntime::shutdown() {
    m_window->close();
    delete m_window;
}

void strobe::internal::RenderBackendRuntime::main(std::shared_ptr<std::binary_semaphore> validRuntimeSignal) {
    std::cout << "started runtime at pid=" << std::this_thread::get_id() << std::endl;
    m_running = true;
    startup();
    validRuntimeSignal->release();

    // don't render on the first frame because there can't be any submissions due to
    // the double buffering latency.
    runtimeBeginFrame();
    runtimeEndFrame();
    m_bufferManager.swapBuffers();
    m_geometrieManager.swapBuffers();
    m_renderObjectQueue.swapBuffers();
    runtimeCompleteFrame();

    double summedFrametime = 0;
    int frameCount = 0;

    while (!m_requestShutdown) {

        GlfwWindow::pollEvents();
        if (m_window->shouldClose())m_requestShutdown = true;
        runtimeBeginFrame();

        using std::chrono::high_resolution_clock;
        using std::chrono::duration_cast;
        using std::chrono::duration;
        using std::chrono::milliseconds;

        auto t1 = high_resolution_clock::now();

        renderloop();

        m_window->swapBuffers();

        auto t2 = high_resolution_clock::now();

        runtimeEndFrame();
        m_bufferManager.swapBuffers();
        m_geometrieManager.swapBuffers();
        m_renderObjectQueue.swapBuffers();
        runtimeCompleteFrame();


        duration<double, std::milli> ms_double = t2 - t1;
        summedFrametime += ms_double.count();
        frameCount += 1;
        if (summedFrametime > 5000) {
            std::cout << "[average RenderBackend frametime : " << (summedFrametime / frameCount) << "ms ]" << std::endl;
            summedFrametime = 0;
            frameCount = 0;
        }
    }
    m_running = false;
    shutdown();

    m_runtimeStoppedSignal.release();

    std::cout << "runtime stopped execution" << std::endl;
}

void strobe::internal::RenderBackendRuntime::renderloop() {

    m_bufferManager.processSubmissions();
    m_geometrieManager.processSubmissions();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // reorder RenderObjectQueue.
    RenderObject* queue = m_renderObjectQueue.getEnqueuedRenderObjects();
    //assert(m_renderObjectQueue.getEnqueuedObjectCount() == 10000);
    for(unsigned int i=0;i<m_renderObjectQueue.getEnqueuedObjectCount();++i){
        const RenderObject& renderObject = queue[i];
        //m_geometrieManager.bind(renderObject.m_geometrieId);

        m_deprecatedShader->bind();
        m_geometrieManager.renderGeometrie(renderObject.m_geometrieId);
    }

    // process RenderObjectQueue.

    //swaps queue buffers (no submissions are allowed during this section).

}

void strobe::internal::RenderBackendRuntime::requestShutdown() {
    m_requestShutdown = true;
}

void strobe::internal::RenderBackendRuntime::awaitShutdown() {
    m_runtimeStoppedSignal.acquire();
}

void strobe::internal::RenderBackendRuntime::beginFrame() {
    if (not m_running)return;
    m_beginFrameSemaphore.release();
}

void strobe::internal::RenderBackendRuntime::endFrame() {
    m_endFrameSemaphore.release();
    using namespace std::chrono_literals;
    bool succ;
    do {
        succ = m_completeFrameSemaphore.try_acquire_for(10ms);
    } while (!succ && m_running);
}

void strobe::internal::RenderBackendRuntime::draw(
        const RenderObject &renderObject) {
    m_renderObjectQueue.enqueue(renderObject);
}


strobe::Buffer strobe::internal::RenderBackendRuntime::createBuffer(
        const std::shared_ptr<ReadSharedMemoryBuffer<char>> &memory,
        Buffer::Usage usage,
        Buffer::Type type) {
    return m_bufferManager.foreignCreate(memory, usage, type);
}

strobe::Geometrie strobe::internal::RenderBackendRuntime::createGeometrie(const std::vector<GeometrieAttribute>
                                                                          &attributes,
                                                                          const std::shared_ptr<GeometrieIndices> &indices) {
    return m_geometrieManager.foreignCreate(attributes, indices);
}

void strobe::internal::RenderBackendRuntime::runtimeBeginFrame() {
    m_beginFrameSemaphore.acquire();
}

void strobe::internal::RenderBackendRuntime::runtimeEndFrame() {
    m_endFrameSemaphore.acquire();
}

void strobe::internal::RenderBackendRuntime::runtimeCompleteFrame() {
    m_completeFrameSemaphore.release();
}
