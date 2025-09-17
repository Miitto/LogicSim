#include "gl/window.hpp"

#include <GLFW/glfw3.h>

namespace gl {
  WindowManager::WindowManager() { glfwInit(); }
  WindowManager::~WindowManager() { glfwTerminate(); }

  Window::Window(int width, int height, const char* title) {
    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
  }

  Window::~Window() { glfwDestroyWindow(window); }

  void Window::makeCurrent() const { glfwMakeContextCurrent(window); }

  bool Window::shouldClose() const { return glfwWindowShouldClose(window); }
  void Window::swapBuffers() const { glfwSwapBuffers(window); }
  void Window::pollEvents() { glfwPollEvents(); }
} // namespace gl