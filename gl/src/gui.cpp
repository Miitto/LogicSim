#include "gl/gui.hpp"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/imgui.h"

namespace gl::gui {
  Context::Context(gl::Window& window)
      : _io((IMGUI_CHECKVERSION(), ImGui::CreateContext(), ImGui::GetIO())) {
    _io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");
  }

  Context::~Context() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
  }

  void Context::newFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
  }
  void Context::endFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  }

  void Context::sleep(int ms) { ImGui_ImplGlfw_Sleep(ms); }

  GuiWindow::GuiWindow(const char* name) { ImGui::Begin(name); }
  GuiWindow::~GuiWindow() { ImGui::End(); }
} // namespace gl::gui