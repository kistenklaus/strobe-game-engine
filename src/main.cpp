
#include "glfw/glfw.h"
#include "vk/vk.h"

int main() {

  strobe::glfw::init();
  strobe::vk::init();

  while(!strobe::glfw::shouldClose()) {
    strobe::glfw::pollEvents();
  }


  strobe::vk::destroy();
  strobe::glfw::destroy();

}
