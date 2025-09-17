#include <iostream>

#include "gl/gl.hpp"

int main() {
  auto& wm = gl::WindowManager::get();

  gl::Window window(800, 600, "Logic Sim", true);
  int version = wm.getGlVersion();

  if (version == 0) {
    std::cerr << "Failed to initialize OpenGL context" << std::endl;
    return -1;
  }

  std::cout << "Loaded OpenGL " << GLVersion.major << "." << GLVersion.minor
            << std::endl;

  glClearColor(.2f, .2f, .2f, 1.f);

  while (!window.shouldClose()) {
    gl::Window::pollEvents();

    glClear(GL_COLOR_BUFFER_BIT);

    window.swapBuffers();
  }

  return 0;
}