#include "input.hpp"
#include "logger.hpp"
#include <gl/gl.hpp>
#include <glm/glm.hpp>
#include <imgui/imgui.h>

int main() {
  Logger::info("Starting application");
  auto& wm = gl::WindowManager::get();

  gl::Window window(800, 600, "Logic Sim", true);
  int version = wm.getGlVersion();

  if (version == 0) {
    Logger::error("Failed to initialize OpenGL context");
    return -1;
  }

  Logger::info("Loaded OpenGL {}.{}\n", GLVersion.major, GLVersion.minor);

  gl::gui::Context gui(window);

  ImVec4 clearColor(.2f, .2f, .2f, 1.f);
  glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);

  Input input(window);
  window.setUserPtr(&input);
  window.setKeyCallback(Input::glfwKeyCallback);

  gl::Program triProgram;
  {
    auto vertexShaderOpt =
        gl::Shader::fromFile("basic_vert.glsl", gl::Shader::VERTEX);
    if (!vertexShaderOpt.has_value()) {
      Logger::error("Failed to load vertex shader");
      return -1;
    }
    auto fragShaderOpt =
        gl::Shader::fromFile("basic_frag.glsl", gl::Shader::FRAGMENT);
    if (!fragShaderOpt.has_value()) {
      Logger::error("Failed to load fragment shader");
      return -1;
    }

    auto& vertexShader = vertexShaderOpt.value();
    auto& fragShader = fragShaderOpt.value();

    triProgram = gl::Program(vertexShader, fragShader);
  }

  struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
  };

  std::array<Vertex, 3> vertices = {
      Vertex{.position = {0.0f, 0.5f, 0.0f}, .color = {1.0f, 0.0f, 0.0f}},
      Vertex{.position = {0.5f, -0.5f, 0.0f}, .color = {0.0f, 1.0f, 0.0f}},
      Vertex{.position = {-0.5f, -0.5f, 0.0f}, .color = {0.0f, 0.0f, 1.0f}},
  };

  gl::Vao vao{};
  gl::Buffer vbo(3 * sizeof(Vertex), &vertices);

  vao.bindVertexBuffer(0, vbo.id(), 0, sizeof(Vertex));
  vao.attribFormat(0, 3, GL_FLOAT, false, offsetof(Vertex, position), 0);
  vao.attribFormat(1, 3, GL_FLOAT, false, offsetof(Vertex, color), 0);

  triProgram.bind();
  vao.bind();

  while (!window.shouldClose()) {
    gl::Window::pollEvents();
    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
      gui.sleep(10);
      continue;
    }
    gui.newFrame();
    input.imGuiWantsMouse(gui.io().WantCaptureMouse);
    input.imGuiWantsKeyboard(gui.io().WantCaptureKeyboard);

    {
      gl::gui::GuiWindow frame("Hello World");

      ImGui::Text("This is some useful text.");

      ImGui::ColorEdit3("clear color", (float*)&clearColor);
    }

    glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    gui.endFrame();
    window.swapBuffers();
  }

  return 0;
}