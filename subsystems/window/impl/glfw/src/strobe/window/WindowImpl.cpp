#include <GLFW/glfw3.h>

#include <strobe/window/WindowImpl.hpp>
#include "strobe/core/memory/PolyAllocator.hpp"

namespace strobe::window {

WindowImpl::WindowImpl(uvec2 size, std::string_view title, PolyAllocatorReference allocator) {
}
WindowImpl::~WindowImpl() {}

}  // namespace strobe::window::details
