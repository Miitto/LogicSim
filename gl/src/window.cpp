#include <glad/glad.h>
// Include GLFW after glad
#include <GLFW/glfw3.h>

#include "gl/window.hpp"

namespace gl {
  WindowManager WindowManager::s_instance;
  WindowManager& WindowManager::get() { return s_instance; }

  WindowManager::WindowManager() { glfwInit(); }
  WindowManager::~WindowManager() { glfwTerminate(); }

  int WindowManager::loadGl() {
    auto& wm = WindowManager::get();
    if (wm.loadedGl)
      return -1;
    int version =
        gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
    if (version != 0) {
      wm.loadedGl = true;
      wm.glLoadedVersion = version;
    }

    return version;
  }

  void WindowManager::glMajor(int major) const {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
  }

  void WindowManager::glMinor(int minor) const {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
  }

  Window::Window(int width, int height, const char* title, bool makeCurrent) {
    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (makeCurrent) {
      this->makeCurrent();
    }
  }

  Window::~Window() { glfwDestroyWindow(window); }

  void Window::makeCurrent() const {
    glfwMakeContextCurrent(window);
    auto& wm = WindowManager::get();
    if (!wm.loadedGl) {
      WindowManager::loadGl();
    }
  }

  bool Window::shouldClose() const { return glfwWindowShouldClose(window); }
  void Window::swapBuffers() const { glfwSwapBuffers(window); }
  void Window::pollEvents() { glfwPollEvents(); }
} // namespace gl
