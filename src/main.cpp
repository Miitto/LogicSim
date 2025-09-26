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

constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 800;

int main() {
  Logger::info("Starting application");
  auto& wm = gl::WindowManager::get();

  gl::Window window(WINDOW_WIDTH, WINDOW_HEIGHT, "Logic Sim FLOAT", true);
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

#pragma region Naive Raymarch Setup
  auto naiveProgramOpt =
      gl::Program::fromFiles({{"naive_vert.glsl", gl::Shader::VERTEX},
                              {"naive_frag.glsl", gl::Shader::FRAGMENT}});
  if (!naiveProgramOpt.has_value()) {
    Logger::error("Failed to load naive program: {}", naiveProgramOpt.error());
    return -1;
  }
  auto& naiveProgram = naiveProgramOpt.value();

  auto toUvProgramOpt =
      gl::Program::fromFiles({{"toUv_vert.glsl", gl::Shader::VERTEX},
                              {"toUv_frag.glsl", gl::Shader::FRAGMENT}});
  if (!toUvProgramOpt.has_value()) {
    Logger::error("Failed to load toUv program: {}", toUvProgramOpt.error());
    return -1;
  }
  auto& toUvProgram = toUvProgramOpt.value();

  auto jumpFloodProgramOpt =
      gl::Program::fromFiles({{"jumpflood_vert.glsl", gl::Shader::VERTEX},
                              {"jumpflood_frag.glsl", gl::Shader::FRAGMENT}});
  if (!jumpFloodProgramOpt.has_value()) {
    Logger::error("Failed to load jumpflood program: {}",
                  jumpFloodProgramOpt.error());
    return -1;
  }
  auto& jumpFloodProgram = jumpFloodProgramOpt.value();

  gl::Texture flipFlopOne{};
  flipFlopOne.storage(1, GL_RGB32F, {WINDOW_WIDTH, WINDOW_HEIGHT});
  flipFlopOne.setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  flipFlopOne.setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  gl::Framebuffer flipFlopOneFbo;
  flipFlopOneFbo.attachTexture(GL_COLOR_ATTACHMENT0, flipFlopOne);

  gl::Texture flipFlopTwo{};
  flipFlopTwo.storage(1, GL_RGB32F, {WINDOW_WIDTH, WINDOW_HEIGHT});
  flipFlopTwo.setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  flipFlopOne.setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  gl::Framebuffer flipFlopTwoFbo;
  flipFlopTwoFbo.attachTexture(GL_COLOR_ATTACHMENT0, flipFlopTwo);

  struct NaiveParams {
    glm::vec2 resolution;
    uint32_t rayCount;
    uint32_t maxSteps;
  };

  gl::Buffer naiveParamsUbo(sizeof(NaiveParams), nullptr,
                            GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT |
                                GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

  auto naiveMapping = naiveParamsUbo.map(
      GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

  struct JfaParams {
    glm::vec2 offset;
  };

  gl::Buffer jfaParamsUbo(sizeof(JfaParams), nullptr,
                          GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT |
                              GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
  auto jfaMapping = jfaParamsUbo.map(GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT |
                                     GL_MAP_COHERENT_BIT);

  uint32_t rayCount = 8;
  uint32_t maxSteps = 256;
  uint32_t jfaPasses =
      static_cast<uint32_t>(ceil(log2(std::max(WINDOW_WIDTH, WINDOW_HEIGHT))));
  uint32_t maxJfaPasses = jfaPasses;
#pragma endregion

  bool showTriangle = false;
  bool showJfa = false;
  bool showToUv = false;

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

      ImGui::ColorEdit4("Clear Color", (float*)&clearColor);
      if (!showTriangle) {
        ImGui::ColorEdit3("Brush Color", &brushColor.r);
        ImGui::SliderFloat("Brush Radius", &brushRadius, 1.f, 20.f);
        ImGui::SliderInt("Ray Count", (int*)&rayCount, 1, 16);
        ImGui::SliderInt("Max Steps", (int*)&maxSteps, 1, 512);

        ImGui::SliderInt("JFA Passes", (int*)&jfaPasses, 1, maxJfaPasses);
        ImGui::Checkbox("Show ToUV", &showToUv);
        ImGui::Checkbox("Show JFA", &showJfa);

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
      glm::vec2 fsize = {static_cast<float>(size.width),
                         static_cast<float>(size.height)};
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

        // Don't care about contents
        flipFlopOne = gl::Texture{};
        flipFlopOne.storage(1, GL_RGBA8, {size.width, size.height});
        flipFlopOneFbo = gl::Framebuffer{};
        flipFlopOneFbo.attachTexture(GL_COLOR_ATTACHMENT0, flipFlopOne);
        flipFlopTwo = gl::Texture{};
        flipFlopTwo.storage(1, GL_RGBA8, {size.width, size.height});
        flipFlopTwoFbo = gl::Framebuffer{};
        flipFlopTwoFbo.attachTexture(GL_COLOR_ATTACHMENT0, flipFlopTwo);

        flipFlopOneFbo.checkStatus();
        flipFlopTwoFbo.checkStatus();

        uint32_t maxPasses =
            static_cast<uint32_t>(ceil(log2(std::max(800, 600))));
        if (jfaPasses > maxPasses) {
          maxJfaPasses = maxPasses;
          jfaPasses = maxPasses;
        }
      }
#pragma endregion

#pragma region Draw
      if (input.mouse().isButtonDown(0)) {
        DrawParams params{
            .from = input.mouse().lastPosition(),
            .to = input.mouse().position,
            .color = {brushColor, brushRadius},
            .resolution = fsize,
        };

        memcpy(drawMapping, &params, sizeof(DrawParams));
        drawParamsBuffer.bindBase(GL_UNIFORM_BUFFER, 0);
        drawFbo.bind();
        drawProgram.bind();
        fullscreenVao.bind();
        glDrawArrays(GL_TRIANGLES, 0, 3);
        gl::Framebuffer::unbind();
      }
#pragma endregion

#pragma region ToUV
      toUvProgram.bind();
      fullscreenVao.bind();
      drawTexture.bind(0);
      flipFlopOneFbo.bind();
      glDrawArrays(GL_TRIANGLES, 0, 3);
#pragma endregion

#pragma region JFA
      if (showJfa && !showToUv) {
        jumpFloodProgram.bind();
        fullscreenVao.bind();
        jfaParamsUbo.bindBase(GL_UNIFORM_BUFFER, 0);

        gl::Texture* inTex = &flipFlopOne;
        gl::Framebuffer* output = &flipFlopTwoFbo;

        glm::vec2 oneOverResolution = {1.f / fsize.x, 1.f / fsize.y};

        for (uint32_t i = 0; i < jfaPasses; i++) {
          float offset = static_cast<float>(pow(2, jfaPasses - i - 1));
          JfaParams jfaParams{
              .offset = offset * oneOverResolution,
          };

          memcpy(jfaMapping, &jfaParams, sizeof(JfaParams));
          inTex->bind(0);
          output->bind();

          glDrawArrays(GL_TRIANGLES, 0, 3);

          if (i % 2 == 0) {
            inTex = &flipFlopTwo;
            output = &flipFlopOneFbo;
          } else {
            inTex = &flipFlopOne;
            output = &flipFlopTwoFbo;
          }
        }
      }
#pragma endregion
      gl::Framebuffer::unbind();

      if (showJfa || showToUv) {
#pragma region Show UV/JFA
        if (jfaPasses % 2 == 1 || showToUv) {
          flipFlopOneFbo.blit(0, 0, 0, size.width, size.height, 0, 0,
                              size.width, size.height, GL_COLOR_BUFFER_BIT,
                              GL_NEAREST);
        } else {
          flipFlopTwoFbo.blit(0, 0, 0, size.width, size.height, 0, 0,
                              size.width, size.height, GL_COLOR_BUFFER_BIT,
                              GL_NEAREST);
        }
#pragma endregion
      } else {
#pragma region Naive Raymarch
        drawTexture.bind(0);
        fullscreenVao.bind();
        naiveProgram.bind();

        NaiveParams nparams{
            .resolution = {fsize.x, fsize.y},
            .rayCount = rayCount,
            .maxSteps = maxSteps,
        };

        memcpy(naiveMapping, &nparams, sizeof(NaiveParams));
        naiveParamsUbo.bindBase(GL_UNIFORM_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 3);
#pragma endregion
      }
    }

    input.frameEnd();
    gui.endFrame();
    window.swapBuffers();
  }

  return 0;
}