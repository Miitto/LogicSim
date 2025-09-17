#pragma once

struct GLFWwindow;

namespace gl {
  /// <summary>
  /// Window Manager that manages the GLFW state. Should be created before any
  /// windows and should persist to after the last one closes.
  /// </summary>
  class WindowManager {
  public:
    WindowManager();
    ~WindowManager();
  };

  class Window {
    GLFWwindow* window;

  public:
    Window(int width, int height, const char* title);
    ~Window();
    void makeCurrent() const;
    bool shouldClose() const;
    void swapBuffers() const;
    static void pollEvents();
  };
} // namespace gl