#include "input.hpp"
#include "logger.hpp"
#include <gl/gl.hpp>
#include <glm/glm.hpp>
#include <imgui/imgui.h>

#include "drawing.hpp"
#include "flatland_rc.hpp"
#include "jfa.hpp"
#include "naive.hpp"
#include "triangle.hpp"

struct BasicVertex {
  glm::vec2 position;
};

std::array<BasicVertex, 3> fullscreenTriangle = {
    BasicVertex{.position = {-1.0f, -1.0f}},
    BasicVertex{.position = {3.0f, -1.0f}},
    BasicVertex{.position = {-1.0f, 3.0f}},
};

constexpr int WINDOW_WIDTH = 1024;
constexpr int WINDOW_HEIGHT = 1024;

enum RenderMode { Triangle, JFA, Distance, Naive, RadianceCascades };
template <> struct fmt::formatter<RenderMode> : formatter<std ::string_view> {
  auto format(const RenderMode& mode, format_context& ctx) const
      -> format_context::iterator {
    std::string_view view = "Gone Wonky";
    switch (mode) {
    case RenderMode::Triangle:
      view = "Triangle";
      break;
    case RenderMode::JFA:
      view = "JFA";
      break;
    case RenderMode::Distance:
      view = "Distance";
      break;
    case RenderMode::Naive:
      view = "Naive";
      break;
    case RenderMode::RadianceCascades:
      view = "Radiance Cascades";
      break;
    }
    return formatter<std::string_view>::format(view, ctx);
  }
};

int main() {
  Logger::info("Starting application");
  auto& wm = gl::WindowManager::get();

  gl::Window window(WINDOW_WIDTH, WINDOW_HEIGHT, "Radiance Cascades FLOAT",
                    true);
  int version = wm.getGlVersion();

  if (version == 0) {
    Logger::error("Failed to initialize OpenGL context");
    return -1;
  }

  Logger::info("Loaded OpenGL {}.{}\n", GLVersion.major, GLVersion.minor);

  Input input(window);

  gl::gui::Context gui(window);

  glm::vec4 clearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);

  gl::BasicBuffer fullscreenVbo(static_cast<GLuint>(fullscreenTriangle.size()) *
                                    sizeof(BasicVertex),
                                fullscreenTriangle.data());
  gl::Vao fullscreenVao;
  fullscreenVao.bindVertexBuffer(0, fullscreenVbo.id(), 0, sizeof(BasicVertex));
  fullscreenVao.attribFormat(0, 2, GL_FLOAT, false, 0, 0);

  uint32_t rayCount = 4;
  uint32_t maxSteps = 32;

  auto oldWindowSize = window.size();
  glm::vec2 fsize = {static_cast<float>(oldWindowSize.width),
                     static_cast<float>(oldWindowSize.height)};

  auto triOpt = Triangle::create(clearColor);
  if (!triOpt.has_value()) {
    Logger::error("Failed to create triangle");
    return -1;
  }
  auto& triangle = triOpt.value();

  auto drawOpt = Drawing::create(fullscreenVao, oldWindowSize);
  if (!drawOpt.has_value()) {
    Logger::error("Failed to create drawing");
    return -1;
  }
  auto& drawing = drawOpt.value();

  auto jfaOpt = Jfa::create(fullscreenVao, window);
  if (!jfaOpt.has_value()) {
    Logger::error("Failed to create JFA");
    return -1;
  }
  auto& jfa = jfaOpt.value();

  auto naiveOpt = NaiveRaymarch::create(fullscreenVao, rayCount, maxSteps);
  if (!naiveOpt.has_value()) {
    Logger::error("Failed to create naive raymarch");
    return -1;
  }
  auto& naive = naiveOpt.value();

  auto flatlandOpt =
      FlatlandRc::create(fullscreenVao, rayCount, maxSteps, oldWindowSize);
  if (!flatlandOpt.has_value()) {
    Logger::error("Failed to create flatland radiance cascades");
    return -1;
  }
  auto& flatland = flatlandOpt.value();

  RenderMode renderMode = RenderMode::RadianceCascades;

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

      ImGui::ColorEdit4("Clear Color", &clearColor.r);
      std::string fmt = fmt::format("{}", renderMode);
      if (ImGui::BeginCombo("Rendering Mode", fmt.c_str())) {
        if (ImGui::Selectable("Triangle", renderMode == RenderMode::Triangle))
          renderMode = RenderMode::Triangle;

        if (ImGui::Selectable("JFA", renderMode == RenderMode::JFA))
          renderMode = RenderMode::JFA;
        if (ImGui::Selectable("Distance", renderMode == RenderMode::Distance))
          renderMode = RenderMode::Distance;
        if (ImGui::Selectable("Naive", renderMode == RenderMode::Naive))
          renderMode = RenderMode::Naive;
        if (ImGui::Selectable("Radiance Cascades",
                              renderMode == RenderMode::RadianceCascades))
          renderMode = RenderMode::RadianceCascades;
        ImGui::EndCombo();
      }

      if (renderMode != RenderMode::Triangle) {
        ImGui::Text("Brush Settings");
        ImGui::ColorEdit3("Brush Color", &drawing.brushColor().r);
        ImGui::SliderFloat("Brush Radius", &drawing.brushRadius(), 1.f, 20.f);

        ImGui::Separator();
        ImGui::Text("Raymarch Settings");
        ImGui::SliderInt("JFA Passes", (int*)&jfa.passes(), 0, jfa.maxPasses());

        if (renderMode != RenderMode::JFA &&
            renderMode != RenderMode::Distance) {
          if (ImGui::SliderInt("Ray Count", (int*)&rayCount, 4,
                               renderMode == RenderMode::Naive ? 128 : 64)) {
            flatland.updateMaxCascades(fsize);
          }
          ImGui::SliderInt("Max Steps", (int*)&maxSteps, 1, 64);

          if (renderMode != RenderMode::Naive) {
            ImGui::SliderInt("Cascade", (int*)&flatland.cascadeIndex(), 0,
                             flatland.maxCascades() - 1);
          }
        }

        if (ImGui::Button("Clear Drawing")) {
          glClearNamedFramebufferiv(drawing.fbo().id(), GL_COLOR, 0,
                                    (GLint*)&clearColor);
        }
      }
    }
#pragma endregion

    if (renderMode == RenderMode::Triangle) {
      triangle.draw();
    } else {
      auto size = window.size();

      // Handle window resize
      if (size != oldWindowSize) {
        Logger::info("Window resize: {}x{}", size.width, size.height);
        oldWindowSize = size;
        fsize = {static_cast<float>(oldWindowSize.width),
                 static_cast<float>(oldWindowSize.height)};

        drawing.resize(size);
        jfa.resize(size);
      }

      drawing.draw(input, fsize);

      jfa.draw(drawing.texture(), size, fsize);

      switch (renderMode) {
      case RenderMode::JFA: {
        jfa.blitToMain(size);
        break;
      }
      case RenderMode::Distance: {
        jfa.blitDistanceToMain(size);
        break;
      }
      case RenderMode::Naive: {
        naive.draw(drawing.texture(), jfa.distanceResult().texture, fsize);
        break;
      }
      case RenderMode::RadianceCascades: {
        flatland.draw(drawing.texture(), jfa.distanceResult().texture, fsize);
        flatland.blitToScreen(size);
        break;
      }
      case RenderMode::Triangle: {
        std::unreachable();
        break;
      }
      }
    }

    input.frameEnd();
    gui.endFrame();
    window.swapBuffers();
  }

  return 0;
}
