#include <glad/glad.h>
// Include GLFW after glad (comment to avoid autosort)
#include <GLFW/glfw3.h>

#include "gl/window.hpp"
#include "logger.hpp"

namespace {
  void GLAPIENTRY debugMessageCallback(GLenum source, GLenum type, GLuint id,
                                       GLenum severity, GLsizei length,
                                       const GLchar* message,
                                       const void* userParam) {
    (void)id;
    (void)length;
    (void)userParam;

    const char* source_str = "";
    switch (source) {
    case GL_DEBUG_SOURCE_API:
      source_str = "API";
      break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
      source_str = "WINDOW_SYSTEM";
      break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
      source_str = "SHADER_COMPILER";
      break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
      source_str = "THIRD_PARTY";
      break;
    case GL_DEBUG_SOURCE_APPLICATION:
      source_str = "APPLICATION";
      break;
    case GL_DEBUG_SOURCE_OTHER:
      source_str = "OTHER";
      break;
    }

    const char* type_str = "";

    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
      type_str = "ERROR";
      break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
      type_str = "DEPRECATED_BEHAVIOR";
      break;

    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
      type_str = "UNDEFINED_BEHAVIOR";
      break;

    case GL_DEBUG_TYPE_PORTABILITY:
      type_str = "PORTABILITY";
      break;
    case GL_DEBUG_TYPE_PERFORMANCE:
      type_str = "PERFORMANCE";
      break;
    case GL_DEBUG_TYPE_MARKER:
      type_str = "MARKER";
      break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
      type_str = "PUSH_GROUP";
      break;
    case GL_DEBUG_TYPE_POP_GROUP:
      type_str = "POP_GROUP";
      break;
    case GL_DEBUG_TYPE_OTHER:
      type_str = "OTHER";
      break;
    }

    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
      gl::Logger::error("GL {} ERROR: ({}) | {}", source_str, type_str,
                        message);
      break;
    case GL_DEBUG_SEVERITY_MEDIUM:
      gl::Logger::warn("GL {} WARNING: ({}) | {}", source_str, type_str,
                       message);
      break;
    case GL_DEBUG_SEVERITY_LOW:
      gl::Logger::info("GL {} INFO: ({}) | {}", source_str, type_str, message);
      break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
      gl::Logger::debug("GL {} DEBUG: ({}) | {}", source_str, type_str,
                        message);
      break;
    default:
      gl::Logger::debug("GL {} UNKNOWN[{}]: ({}) | {}", source_str, severity,
                        type_str, message);
      break;
    }
  }
} // namespace

namespace gl {
  WindowManager WindowManager::s_instance;
  WindowManager& WindowManager::get() { return s_instance; }

  WindowManager::WindowManager() {
    if (glfwInit()) {
      gl::Logger::debug("Window Manager initialized");
#ifndef NDEBUG
      glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
    }
  }
  WindowManager::~WindowManager() { glfwTerminate(); }

  int WindowManager::loadGl() {
    auto& wm = WindowManager::get();
    if (wm.loadedGl) {
      gl::Logger::warn("Attempted to load OpenGL multiple times");
      return -1;
    }
    int version =
        gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
    if (version != 0) {
      wm.loadedGl = true;
      wm.glLoadedVersion = version;
    } else {
      gl::Logger::error("Failed to load OpenGL");
    }

#ifndef NDEBUG
    glEnable(GL_DEBUG_OUTPUT);
    // Allows to breakpoint on GL errors and have the callstack be correct
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(debugMessageCallback, 0);
    gl::Logger::debug("Attached debug message callback");
#endif

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

  void Window::setUserPtr(void* ptr) { glfwSetWindowUserPointer(window, ptr); }
  void* Window::getUserPtr(GLFWwindow* window) {
    return glfwGetWindowUserPointer(window);
  }

  Window::Size Window::size() const {
    Size s;
    glfwGetWindowSize(window, &s.width, &s.height);
    return s;
  }

} // namespace gl
