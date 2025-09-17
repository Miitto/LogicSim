#pragma once

struct GLFWwindow;

namespace gl {
  /// <summary>
  /// An individual window. Requires a <c>WindowManager</c> to be created first.
  /// </summary>
  class Window {
    GLFWwindow* window;

  public:
    /// <summary>
    /// Constructs a Window object with the specified dimensions and title.
    /// </summary>
    /// <param name="width">The width of the window in pixels.</param>
    /// <param name="height">The height of the window in pixels.</param>
    /// <param name="title">The title of the window.</param>
    /// <param name="makeCurrent">Whether to make the window the current context
    /// upon creation. Defaults to false.</param>
    Window(int width, int height, const char* title, bool makeCurrent = false);
    ~Window();

    void makeCurrent() const;
    bool shouldClose() const;
    void swapBuffers() const;
    static void pollEvents();
  };

  /// <summary>
  /// Window Manager that manages the GLFW state. Should be created before any
  /// windows and should persist to after the last one closes.
  /// </summary>
  class WindowManager {
    bool loadedGl = false;
    int glLoadedVersion = 0;

    static gl::WindowManager s_instance;

    static int loadGl();

    WindowManager();

  public:
    ~WindowManager();

    static WindowManager& get();

    int getGlVersion() { return glLoadedVersion; }

    void glMajor(int major) const;
    void glMinor(int minor) const;
    void glVersion(int major, int minor) const {
      glMajor(major);
      glMinor(minor);
    }

    friend class Window;
  };

} // namespace gl
