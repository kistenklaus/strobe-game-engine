#pragma once

#include <memory>
#include "renderobj/RenderObject.h"
#include "./buffer/Buffer.h"
#include <vector>
#include <strobe/shared_memory_buffer.h>
#include <glm/glm.hpp>
#include <semaphore>
#include "./geometrie/GeometrieIndices.h"
#include "./geometrie/Geometrie.h"
#include "./geometrie/GeometrieAttribute.h"

namespace strobe {
    namespace internal {
        class RenderBackendRuntime;
    }

    class RenderBackend {
    public:
        explicit RenderBackend();

        ~RenderBackend();

        std::shared_ptr<std::binary_semaphore> start();

        void stop();

        void awaitShutdown();

        void beginFrame();

        void endFrame();

        bool isRunning();

        void draw(const RenderObject& renderObject);;


        template<typename T>
        Buffer createBuffer(const std::shared_ptr<ReadSharedMemoryBuffer<T>> &memory,
                            Buffer::Usage usage,
                            Buffer::Type type) {
            if constexpr (std::is_same<T, char>()) {
                return createBufferInternal(memory, usage, type);
            } else {
                return createBufferInternal(
                        std::make_shared<CastedReadSharedMemoryBuffer<char, T>>(memory),
                        usage,
                        type);
            }
        }


        Geometrie createGeometrie(const std::vector<GeometrieAttribute> &attributes,
                                  const std::shared_ptr<GeometrieIndices> &indices = nullptr);

        Geometrie createGeometrie(const std::vector<GeometrieAttribute> &attributes,
                                  const GeometrieIndices &indices);

        Geometrie createGeometrie(const GeometrieAttribute &attrib1,
                                  const GeometrieAttribute &attrib2,
                                  const GeometrieAttribute &attrib3,
                                  const std::shared_ptr<GeometrieIndices> &indices = nullptr);

        Geometrie createGeometrie(const GeometrieAttribute &attrib1,
                                  const GeometrieAttribute &attrib2,
                                  const GeometrieAttribute &attrib3,
                                  const GeometrieIndices &indices);


        Geometrie createGeometrie(const GeometrieAttribute &attrib1,
                                  const GeometrieAttribute &attrib2,
                                  const std::shared_ptr<GeometrieIndices> &indices);

        Geometrie createGeometrie(const GeometrieAttribute &attrib1,
                                  const GeometrieAttribute &attrib2,
                                  const GeometrieIndices &indices);

        Geometrie createGeometrie(const GeometrieAttribute &attrib1,
                                  const std::shared_ptr<GeometrieIndices> &indices = nullptr);

        Geometrie createGeometrie(const GeometrieAttribute &attrib1,
                                  const GeometrieIndices &indices);

    private:

        Buffer createBufferInternal(const std::shared_ptr<ReadSharedMemoryBuffer<char>> &memory,
                                    Buffer::Usage usage,
                                    Buffer::Type type);

    private:

        internal::RenderBackendRuntime *m_runtime = nullptr;
    };

}