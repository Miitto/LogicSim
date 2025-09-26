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

  gl::Window window(800, 600, "Logic Sim FLOAT", true);
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

#pragma region Tri Setup
  auto triProgramOpt =
      gl::Program::fromFiles({{"basic_vert.glsl", gl::Shader::VERTEX},
                              {"basic_frag.glsl", gl::Shader::FRAGMENT}});
  if (!triProgramOpt.has_value()) {
    Logger::error("Failed to load basic program: {}", triProgramOpt.error());
    return -1;
  }
  auto& triProgram = triProgramOpt.value();

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
#pragma endregion

#pragma region Drawing Setup
  auto drawProgramOpt =
      gl::Program::fromFiles({{"draw_vert.glsl", gl::Shader::VERTEX},
                              {"draw_frag.glsl", gl::Shader::FRAGMENT}});
  if (!drawProgramOpt.has_value()) {
    Logger::error("Failed to load draw program: {}", drawProgramOpt.error());
    return -1;
  }
  auto& drawProgram = drawProgramOpt.value();

  gl::Texture drawTexture{};
  drawTexture.storage(1, GL_RGBA8, {800, 600});
  gl::Framebuffer drawFbo;
  drawFbo.attachTexture(GL_COLOR_ATTACHMENT0, drawTexture);

  gl::Buffer fullscreenVbo(static_cast<GLuint>(fullscreenTriangle.size()) *
                               sizeof(BasicVertex),
                           fullscreenTriangle.data());
  gl::Vao fullscreenVao;
  fullscreenVao.bindVertexBuffer(0, fullscreenVbo.id(), 0, sizeof(BasicVertex));
  fullscreenVao.attribFormat(0, 2, GL_FLOAT, false, 0, 0);

  struct DrawParams {
    glm::vec2 from;
    glm::vec2 to;
    glm::vec4 color;
    glm::vec2 resolution;
  };

  gl::Buffer drawParamsBuffer(sizeof(DrawParams), nullptr,
                              GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT |
                                  GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

  auto drawMapping = drawParamsBuffer.map(
      GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

  float brushRadius = 5.f;
  glm::vec3 brushColor{1.f, 0.f, 0.f};
#pragma endregion

  auto naiveProgramOpt =
      gl::Program::fromFiles({{"naive_vert.glsl", gl::Shader::VERTEX},
                              {"naive_frag.glsl", gl::Shader::FRAGMENT}});
  if (!naiveProgramOpt.has_value()) {
    Logger::error("Failed to load naive program: {}", naiveProgramOpt.error());
    return -1;
  }
  auto& naiveProgram = naiveProgramOpt.value();
  (void)naiveProgram;

  bool showTriangle = false;

  while (!window.shouldClose()) {
    gl::Window::pollEvents();
    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
      gui.sleep(10);
      continue;
    }
    gui.newFrame();
    input.imGuiWantsMouse(gui.io().WantCaptureMouse);
    input.imGuiWantsKeyboard(gui.io().WantCaptureKeyboard);

#pragma region ImGUI
    {
      gl::gui::GuiWindow frame("Scene Controls");
      ImGui::Checkbox("Triangle", &showTriangle);

      ImGui::ColorEdit3("Clear Color", (float*)&clearColor);
      if (!showTriangle) {
        ImGui::ColorEdit3("Brush Color", &brushColor.r);
        ImGui::SliderFloat("Brush Radius", &brushRadius, 1.f, 20.f);
        ImGui::Button("Clear");
        if (ImGui::IsItemClicked()) {
          glClearNamedFramebufferiv(drawFbo.id(), GL_COLOR, 0,
                                    (GLint*)&clearColor);
        }
      }
    }
#pragma endregion

    if (showTriangle) {
      glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
      glClear(GL_COLOR_BUFFER_BIT);
      triProgram.bind();
      vao.bind();

      glDrawArrays(GL_TRIANGLES, 0, 3);
    } else {
      auto size = window.size();
#pragma region Framebuffer Resize
      auto framebufferSize = drawTexture.size();
      if (size.width != framebufferSize.width ||
          size.height != framebufferSize.height) {
        Logger::info("Resizing framebuffer to {}x{}", size.width, size.height);
        gl::Texture newTexture{};
        newTexture.storage(1, GL_RGBA8, {size.width, size.height});
        gl::Framebuffer newFbo{};
        newFbo.attachTexture(GL_COLOR_ATTACHMENT0, newTexture);

        drawFbo.blit(newFbo.id(), 0, 0, framebufferSize.width,
                     framebufferSize.height, 0, 0, size.width, size.height,
                     GL_COLOR_BUFFER_BIT, GL_LINEAR);
        drawTexture = std::move(newTexture);
        drawFbo = std::move(newFbo);
      }
#pragma endregion

      if (input.mouse().isButtonDown(0)) {
        glm::vec2 winSize{static_cast<float>(size.width),
                          static_cast<float>(size.height)};
        DrawParams params{
            .from = input.mouse().lastPosition(),
            .to = input.mouse().position,
            .color = {brushColor, brushRadius},
            .resolution = winSize,
        };

        memcpy(drawMapping, &params, sizeof(DrawParams));
        drawParamsBuffer.bindBase(GL_UNIFORM_BUFFER, 0);
        drawFbo.bind();
        drawProgram.bind();
        fullscreenVao.bind();
        glDrawArrays(GL_TRIANGLES, 0, 3);

        gl::Framebuffer::unbind();
      }
      drawFbo.blit(0, 0, 0, size.width, size.height, 0, 0, size.width,
                   size.height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    }

    input.frameEnd();
    gui.endFrame();
    window.swapBuffers();
  }

  return 0;
}