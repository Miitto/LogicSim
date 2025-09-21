#pragma once

#include "gl/window.hpp"
#include <functional>

struct ImGuiIO;

namespace gl::gui {
  class Context {
    ImGuiIO& _io;

  public:
    Context(gl::Window& window);
    ~Context();

    void newFrame();
    void endFrame();

    ImGuiIO& io() { return _io; }

    void sleep(int ms);
  };

  class GuiWindow {
  public:
    GuiWindow(const char* name);
    ~GuiWindow();
  };

} // namespace gl::gui