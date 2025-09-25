#include "input.hpp"
#include "logger.hpp"
#include <gl/gl.hpp>
#include <gl\framebuffer.hpp>
#include <gl\texture.hpp>
#include <glm/glm.hpp>
#include <imgui/imgui.h>

struct BasicVertex {
  glm::vec2 position;
};

std::array<BasicVertex, 3> fullscreenTriangle = {
    BasicVertex{.position = {-1.0f, -1.0f}},
    BasicVertex{.position = {3.0f, -1.0f}},
    BasicVertex{.position = {-1.0f, 3.0f}},
};

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

  Input input(window);

  gl::gui::Context gui(window);

  ImVec4 clearColor(.2f, .2f, .2f, 1.f);
  glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);

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

  gl::Program drawProgram;
  {
    auto vertexShaderOpt =
        gl::Shader::fromFile("draw_vert.glsl", gl::Shader::VERTEX);
    if (!vertexShaderOpt.has_value()) {
      Logger::error("Failed to load vertex shader");
      return -1;
    }
    auto fragShaderOpt =
        gl::Shader::fromFile("draw_frag.glsl", gl::Shader::FRAGMENT);
    if (!fragShaderOpt.has_value()) {
      Logger::error("Failed to load fragment shader");
      return -1;
    }

    auto& vertexShader = vertexShaderOpt.value();
    auto& fragShader = fragShaderOpt.value();

    drawProgram = gl::Program(vertexShader, fragShader);
  }

  struct TriVertex {
    glm::vec3 position;
    glm::vec3 color;
  };

  std::array<TriVertex, 3> vertices = {
      TriVertex{.position = {0.0f, 0.5f, 0.0f}, .color = {1.0f, 0.0f, 0.0f}},
      TriVertex{.position = {0.5f, -0.5f, 0.0f}, .color = {0.0f, 1.0f, 0.0f}},
      TriVertex{.position = {-0.5f, -0.5f, 0.0f}, .color = {0.0f, 0.0f, 1.0f}},
  };

  gl::Vao vao{};
  gl::Buffer vbo(3 * sizeof(TriVertex), &vertices);

  vao.bindVertexBuffer(0, vbo.id(), 0, sizeof(TriVertex));
  vao.attribFormat(0, 3, GL_FLOAT, false, offsetof(TriVertex, position), 0);
  vao.attribFormat(1, 3, GL_FLOAT, false, offsetof(TriVertex, color), 0);

  bool showTriangle = false;

  struct DrawParams {
    glm::vec2 from;
    glm::vec2 to;
    glm::vec4 color;
  };

  gl::Buffer drawParamsBuffer(sizeof(DrawParams), nullptr,
                              GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT |
                                  GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

  auto drawMapping = drawParamsBuffer.map(
      GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
  (void)drawMapping;

  gl::Texture drawTexture{};
  drawTexture.storage(1, GL_RGBA8, 800, 600);
  // drawTexture.subImage(0, 0, 0, 800, 600, GL_RGBA, GL_UNSIGNED_BYTE,
  // nullptr);
  gl::Framebuffer drawFbo;
  drawFbo.attachTexture(GL_COLOR_ATTACHMENT0, drawTexture);

  gl::Buffer fullscreenVbo(static_cast<GLuint>(fullscreenTriangle.size()) *
                               sizeof(BasicVertex),
                           fullscreenTriangle.data());
  gl::Vao fullscreenVao;
  fullscreenVao.bindVertexBuffer(0, fullscreenVbo.id(), 0, sizeof(BasicVertex));
  fullscreenVao.attribFormat(0, 2, GL_FLOAT, false, 0, 0);

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

      ImGui::Checkbox("ShowCircle", &showTriangle);
    }

    glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT);

    if (showTriangle) {
      triProgram.bind();
      vao.bind();

      glDrawArrays(GL_TRIANGLES, 0, 3);
    } else {
      if (input.mouse().isButtonDown(0)) {
        auto currentPos = input.mouse().position;
        auto lastPos = input.mouse().lastPosition();
        Logger::debug("Drawing from {},{} to {},{}", lastPos.x, lastPos.y,
                      currentPos.x, currentPos.y);
        DrawParams params{
            .from = input.mouse().lastPosition() / glm::vec2{800., 600.},
            .to = input.mouse().position / glm::vec2{800., 600.},
            .color = {1.f, 0.f, 0.f, 0.01f},
        };

        memcpy(drawMapping, &params, sizeof(DrawParams));
        drawParamsBuffer.bindBase(GL_UNIFORM_BUFFER, 0);
        drawFbo.bind();
        drawProgram.bind();
        fullscreenVao.bind();
        glDrawArrays(GL_TRIANGLES, 0, 3);

        gl::Framebuffer::unbind();
      }
      drawFbo.blit(0, 0, 800, 600, 0, 0, 800, 600, 0, GL_COLOR_BUFFER_BIT,
                   GL_LINEAR);
    }

    input.frameEnd();
    gui.endFrame();
    window.swapBuffers();
  }

  return 0;
}