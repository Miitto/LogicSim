#include <iostream>
#include <print>

#include "gl/gl.hpp"
#include "imgui/imgui.h"

int main() {

  /*auto fontNames = text::getFontNames();
  std::print("Font Names:\n");

  for (auto& name : fontNames) {
    std::println("\t{}", name);
  }*/

  auto& wm = gl::WindowManager::get();

  gl::Window window(800, 600, "Logic Sim", true);
  int version = wm.getGlVersion();

  if (version == 0) {
    std::cerr << "Failed to initialize OpenGL context" << std::endl;
    return -1;
  }

  std::print("Loaded OpenGL {}.{}\n", GLVersion.major, GLVersion.minor);

  gl::gui::Context gui(window);

  glClearColor(.2f, .2f, .2f, 1.f);

  while (!window.shouldClose()) {
    gl::Window::pollEvents();
    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
      gui.sleep(10);
      continue;
    }
    gui.newFrame();
    auto wantsMouse = gui.io().WantCaptureMouse;
    auto wantsKeyboard = gui.io().WantCaptureKeyboard;
    (void)wantsMouse;
    (void)wantsKeyboard;

    {
      gl::gui::GuiWindow frame("Hello World");

      ImGui::Text("This is some useful text.");
    }

    glClear(GL_COLOR_BUFFER_BIT);

    gui.endFrame();
    window.swapBuffers();
  }

  return 0;
}