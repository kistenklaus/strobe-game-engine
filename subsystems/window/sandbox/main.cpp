#include <strobe/core/memory/Mallocator.hpp>
#include <strobe/window/WindowSubsystem.hpp>

int main() {
  strobe::WindowSubsystem<strobe::Mallocator> ws{{400, 600}, "abc"};
}
