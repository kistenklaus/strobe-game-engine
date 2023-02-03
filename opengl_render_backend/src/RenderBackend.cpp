#include "RenderBackend.h"
#include "RenderBackendRuntime.h"
#include <cassert>
#include <iostream>

strobe::RenderBackend::RenderBackend()
        : m_runtime(nullptr) {}

strobe::RenderBackend::~RenderBackend() {
    stop();
}

std::shared_ptr<std::binary_semaphore> strobe::RenderBackend::start() {
    if (m_runtime != nullptr && m_runtime->isRunning()) {
        throw std::logic_error("Calling start on a running RenderBackend is not allowed!");
    }

    std::shared_ptr<std::binary_semaphore> validRuntimeSignal =
            std::make_shared<std::binary_semaphore>(0);

    if (m_runtime == nullptr) {
        m_runtime = new RenderBackendRuntime(validRuntimeSignal);
    }
    return validRuntimeSignal;
}


void strobe::RenderBackend::stop() {
    delete m_runtime;
    m_runtime = nullptr;
}

void strobe::RenderBackend::awaitShutdown() {
    if (m_runtime == nullptr) {
        throw std::logic_error("Can't await the external stop of RenderBackend because it is not running "
                               "[suggestion to enable parameter waitForValidRuntime when start]");
    }
    m_runtime->awaitShutdown();
    stop();
}

void strobe::RenderBackend::beginFrame() {
    assert(m_runtime);
    m_runtime->beginFrame();
}

void strobe::RenderBackend::endFrame() {
    assert(m_runtime);
    m_runtime->endFrame();
}

bool strobe::RenderBackend::isRunning() {
    if (m_runtime == nullptr)return false;
    return m_runtime->isRunning();
}


strobe::Buffer strobe::RenderBackend::createBuffer(
        const std::shared_ptr<ReadSharedMemoryBuffer<char>> &memory,
        Buffer::Usage usage,
        Buffer::Type type) {
    assert(m_runtime);
    return m_runtime->createBuffer(memory, usage, type);
}

strobe::Geometrie strobe::RenderBackend::createGeometrie(
        const std::vector<GeometrieAttribute> &attributes,
        const std::shared_ptr<GeometrieIndices> &indices) {
    assert(m_runtime);
    return m_runtime->createGeometrie(attributes, indices);
}

strobe::Geometrie strobe::RenderBackend::createGeometrie(const std::vector<GeometrieAttribute> &attributes,
                                                         const strobe::GeometrieIndices &indices) {
    assert(m_runtime);
    return m_runtime->createGeometrie(attributes, std::make_shared<GeometrieIndices>(indices));
}

strobe::Geometrie strobe::RenderBackend::createGeometrie(
        const GeometrieAttribute &attrib1,
        const GeometrieAttribute &attrib2,
        const GeometrieAttribute &attrib3,
        const std::shared_ptr<GeometrieIndices> &indices) {
    assert(m_runtime);
    return m_runtime->createGeometrie({attrib1, attrib2, attrib3}, indices);
}

strobe::Geometrie strobe::RenderBackend::createGeometrie(
        const GeometrieAttribute &attrib1,
        const GeometrieAttribute &attrib2,
        const GeometrieAttribute &attrib3,
        const GeometrieIndices &indices) {
    assert(m_runtime);
    return m_runtime->createGeometrie({attrib1, attrib2, attrib3},
                                      std::make_shared<GeometrieIndices>(indices));
}

strobe::Geometrie strobe::RenderBackend::createGeometrie(
        const GeometrieAttribute &attrib1,
        const GeometrieAttribute &attrib2,
        const std::shared_ptr<GeometrieIndices> &indices) {
    assert(m_runtime);
    return m_runtime->createGeometrie({attrib1, attrib2}, indices);
}


strobe::Geometrie strobe::RenderBackend::createGeometrie(
        const strobe::GeometrieAttribute &attrib1,
        const strobe::GeometrieAttribute &attrib2,
        const strobe::GeometrieIndices &indices) {
    assert(m_runtime);
    return m_runtime->createGeometrie({attrib1, attrib2},
                                      std::make_shared<GeometrieIndices>(indices));
}

strobe::Geometrie strobe::RenderBackend::createGeometrie(
        const GeometrieAttribute &attrib1,
        const std::shared_ptr<GeometrieIndices> &indices) {
    assert(m_runtime);
    return m_runtime->createGeometrie({attrib1}, indices);
}

strobe::Geometrie strobe::RenderBackend::createGeometrie(
        const GeometrieAttribute &attrib1,
        const GeometrieIndices &indices) {
    assert(m_runtime);
    return m_runtime->createGeometrie({attrib1},
                                      std::make_shared<GeometrieIndices>(indices));
}
