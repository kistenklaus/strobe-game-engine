#include <strobe/core/memory/Mallocator.hpp>
#include <strobe/window/WindowSubsystem.hpp>

int main() {
  strobe::window::WindowSubsystem<strobe::Mallocator> ws{{400, 600}, "abc"};
}
