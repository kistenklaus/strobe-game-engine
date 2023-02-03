
#include <strobe/renderer_backend.h>
#include <iostream>
#include <thread>
#include <vector>

int main() {

    float positionData[] = {
            -0.5f, -0.5f, 0,
            -0.5f, 0.5f, 0,
            0.5f, 0.5f, 0,
            -0.5f, -0.5f, 0
    };

    int indicesData[] = {
            0, 1, 2,
            2, 3, 0
    };

    std::shared_ptr<strobe::ConstSharedMemoryBuffer<char, 12 * sizeof(float)>>
            vertexMemory =
            std::make_shared<strobe::ConstSharedMemoryBuffer<char, 12 * sizeof(float)>>(
                    reinterpret_cast<char *>(&positionData[0]));

    std::shared_ptr<strobe::ConstSharedMemoryBuffer<char, 12 * sizeof(float)>>
            indicesMemory =
            std::make_shared<strobe::ConstSharedMemoryBuffer<char, 12 * sizeof(float)>>(
                    reinterpret_cast<char *>(&indicesData[0]));


    std::cout << "started main at pid=" << std::this_thread::get_id() << std::endl;

    strobe::RenderBackend renderBackend;


    auto backendInitSignal = renderBackend.start();
    backendInitSignal->acquire(); // await renderer to finish initalization.
    strobe::Buffer vertexBuffer = renderBackend.createBuffer(vertexMemory,
                                                             strobe::Buffer::Usage::STATIC_DRAW,
                                                             strobe::Buffer::Type::VERTEX_BUFFER);
    strobe::Buffer elementBuffer = renderBackend.createBuffer(indicesMemory,
                                                              strobe::Buffer::Usage::STATIC_DRAW,
                                                              strobe::Buffer::Type::ELEMENT_BUFFER);

    {
        // create the same element because both point to the same memory and are equal in all other states.
        strobe::Buffer elementBuffer2 = renderBackend.createBuffer(indicesMemory,
                                                                   strobe::Buffer::Usage::STATIC_DRAW,
                                                                   strobe::Buffer::Type::ELEMENT_BUFFER);
    }

    strobe::GeometrieAttribute positionAttribute(0,
                                                 strobe::GeometrieAttribute::Type::Vec4_32f,
                                                 vertexBuffer,
                                                 sizeof(float) * 4,
                                                 0);

    strobe::GeometrieIndices indices(elementBuffer,
                                     strobe::GeometrieIndexType::U32);

    strobe::Geometrie geometrie = renderBackend.createGeometrie(positionAttribute);
    strobe::Geometrie geometrie2 = renderBackend.createGeometrie(positionAttribute);

    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;

    double summedFrametime = 0;
    int frameCount = 0;

    while (renderBackend.isRunning()) {
        using namespace std::chrono_literals;
        renderBackend.beginFrame();
        auto t1 = high_resolution_clock::now();
        // submit to render backend.
        // update double buffered resources.
        renderBackend.endFrame();
        // update single buffered data.
        // swap buffers.
        // don't update double buffered resources after swap buffers. Duh!


        auto t2 = high_resolution_clock::now();
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