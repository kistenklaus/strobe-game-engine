
#include <strobe/renderer_backend.h>
#include <iostream>
#include <thread>
#include <vector>
#include <cstddef>
#include <Resource.h>

int main() {
    Resource resource = LOAD_RESOURCE(simple_fs_glsl);
    std::cout << std::string(resource.data(), resource.size()) << std::endl;

    std::cout << "started main at pid=" << std::this_thread::get_id() << std::endl;

    auto vertexMemory =
            std::make_shared<strobe::ConstSharedMemoryBuffer<float, 12>>(
                    std::vector<float>{
                            -0.5f, -0.5f, 0.0f,
                            -0.5f, 0.5f, 0.0f,
                            0.5f, 0.5f, 0.0f,
                            0.5f, -0.5f, 0.0f
                    });

    auto indicesMemory =
            std::make_shared<strobe::ConstSharedMemoryBuffer<unsigned int, 6>>(
                    std::vector<unsigned int>{
                            0, 1, 2,
                            2, 3, 0
                    });


    strobe::RenderBackend renderBackend;

    renderBackend.start()->acquire();

    strobe::Buffer vertexBuffer = renderBackend.createBuffer(
            //idk this cast is required for some annoying reason.
            std::static_pointer_cast<strobe::ReadSharedMemoryBuffer<float>>(vertexMemory),
            strobe::Buffer::Usage::STATIC_DRAW,
            strobe::Buffer::Type::VERTEX_BUFFER);

    strobe::Buffer elementBuffer = renderBackend.createBuffer(
            std::static_pointer_cast<strobe::ReadSharedMemoryBuffer<unsigned int>>(indicesMemory),
            strobe::Buffer::Usage::STATIC_DRAW,
            strobe::Buffer::Type::ELEMENT_BUFFER);


    strobe::Geometrie geometrie = renderBackend.createGeometrie(
            strobe::GeometrieAttribute(0,
                                       strobe::GeometrieAttribute::Type::Vec3_32f,
                                       vertexBuffer,
                                       sizeof(float) * 3,
                                       0),
            strobe::GeometrieIndices(elementBuffer, strobe::GeometrieIndexType::U32));

    strobe::RenderObject renderObject;
    renderObject.m_geometrieId = geometrie.id();

    // TODO Memory Barrier.

    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;

    double summedFrametime = 0;
    int frameCount = 0;

    while (renderBackend.isRunning()) {
        renderBackend.beginFrame();
        using namespace std::chrono_literals;
        auto t1 = high_resolution_clock::now();
        // submit to render backend.
        for(unsigned int i=0;i<10000;i++){
            renderBackend.draw(renderObject);
        }
        // update double buffered resources.
        auto t2 = high_resolution_clock::now();
        renderBackend.endFrame();
        // update single buffered data.
        // swap buffers.
        // don't update double buffered resources after swap buffers. Duh!


        duration<double, std::milli> ms_double = t2 - t1;
        summedFrametime += ms_double.count();
        frameCount += 1;
        if (summedFrametime > 5000) {
            std::cout << "[average frametime : " << (summedFrametime / frameCount) << "ms ]" << std::endl;
            summedFrametime = 0;
            frameCount = 0;
        }
    }

    renderBackend.awaitShutdown();


    std::cout << "main stopped execution" << std::endl;

}