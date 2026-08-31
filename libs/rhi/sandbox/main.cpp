
#include "io.hpp"
#include "strobe/core/lina/vec.hpp"
#include "strobe/window/window_impl.hpp"
#include <GLFW/glfw3.h>
#include <tracy/Tracy.hpp>
#include <unistd.h>
#include <vulkan/vulkan_core.h>

#include <strobe/rhi/rhi.hpp>

using namespace strobe;

int main() {
  Platform::start([]() {
    window::WindowImpl window{uvec2{800, 600}, "strobe"};

    rhi::Device device = strobe::rhi::create_device({
        .debug_utils = true,
    });

  });
}
