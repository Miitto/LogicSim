#include <iostream>
#include <print>

#include "gl/gl.hpp"
#include "imgui/imgui.h"

#include "logger.hpp"
#include <gl\buffer.hpp>
#include <gl\shaders.hpp>
#include <gl\vao.hpp>

#include <glm/glm.hpp>

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

  gl::Program triProgram;
  {
    gl::Shader vertexShader(gl::Shader::VERTEX,
                            R"(
        #version 460 core
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec3 color;

        layout(location = 0) out vec3 fragColor;

        void main()
        {
            gl_Position = vec4(aPos, 1.0);
        }
    )");

    gl::Shader fragShader(gl::Shader::FRAGMENT, R"(
        #version 330 core
        out vec4 FragColor;
        layout(location = 0) in vec3 fragColor;
        void main()
        {
            FragColor = vec4(fragColor, 1.0);
        }
    )");

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

  vao.bind();
  glVertexAttributePointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                           (void*)offsetof(Vertex, position));
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void*)offsetof(Vertex, color));
  glEnableVertexAttribArray(0);

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

      ImGui::ColorEdit3("clear color", (float*)&clearColor);
    }

    glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT);

    gui.endFrame();
    window.swapBuffers();
  }

  return 0;
}