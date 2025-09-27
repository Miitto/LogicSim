#pragma once

#include "flipFlops.hpp"
#include <gl/gl.hpp>
#include <glm/glm.hpp>
#include <optional>
#include <vector>

class FlatlandRc {
  const gl::Vao& m_fullscreenVao;

  gl::Program m_program;

  TexFbo m_result;

  gl::Buffer m_constantsUbo;
  std::vector<gl::Buffer> m_paramsUbo;

  FlipFlops m_flipFlops;

  const uint32_t& m_baseRayCount;
  const uint32_t& m_maxSteps;

  uint32_t m_cascadeIndex = 0;
  uint32_t m_maxCascades;

  FlatlandRc(const gl::Vao& fullscreenVao, gl::Program&& rcProgram,
             TexFbo&& result, gl::Buffer&& constantsUbo,
             std::vector<gl::Buffer>&& paramsUbo, FlipFlops&& flipFlops,
             const uint32_t& rayCount, const uint32_t& maxSteps,
             uint32_t maxCascades)
      : m_fullscreenVao(fullscreenVao), m_program(std::move(rcProgram)),
        m_result(std::move(result)), m_constantsUbo(std::move(constantsUbo)),
        m_paramsUbo(std::move(paramsUbo)), m_flipFlops(std::move(flipFlops)),
        m_baseRayCount(rayCount), m_maxSteps(maxSteps),
        m_maxCascades(maxCascades) {}

public:
  const uint32_t& maxCascades() const { return m_maxCascades; }

  static uint32_t calcMaxCascades(const glm::vec2& fsize,
                                  uint32_t baseRayCount) {
    double diagonal = sqrt(fsize.x * fsize.x + fsize.y * fsize.y);
    double cascades = ceil(log(diagonal) / log(baseRayCount)) + 1;
    return static_cast<uint32_t>(cascades);
  }

  void updateMaxCascades(const glm::vec2& fsize) {
    uint32_t maxCascades = calcMaxCascades(fsize, m_baseRayCount);
    if (m_cascadeIndex >= maxCascades) {
      m_cascadeIndex = maxCascades - 1;
    }

    if (maxCascades > m_maxCascades) {
      // Add more UBOs
      for (uint32_t i = m_maxCascades; i < maxCascades; i++) {
        gl::Buffer ubo(sizeof(FlatlandRcParams), nullptr,
                       GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT |
                           GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
        ubo.map(GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
        m_paramsUbo.push_back(std::move(ubo));
      }
    } else if (maxCascades < m_maxCascades) {
      // Remove extra UBOs
      m_paramsUbo.resize(maxCascades);
    }

    m_flipFlops = FlipFlops(
        GL_RGBA32F, {static_cast<int>(fsize.x), static_cast<int>(fsize.y)},
        maxCascades);

    m_maxCascades = maxCascades;
  }

  // Constant between iterations
  struct FlatlandRcConstants {
    glm::vec2 resolution;
    uint32_t baseRayCount;
    uint32_t maxSteps;
    uint32_t maxCascades;
  };

  // Per iteration
  struct FlatlandRcParams {
    uint32_t currentCascade;
  };

  const uint32_t& cascadeIndex() const { return m_cascadeIndex; }

  static std::optional<FlatlandRc> create(const gl::Vao& fullscreenVao,
                                          const uint32_t& rayCount,
                                          const uint32_t& maxSteps,
                                          const gl::Window::Size& size) {

    glm::vec2 fsize{static_cast<float>(size.width),
                    static_cast<float>(size.height)};

    uint32_t maxCascades = calcMaxCascades(fsize, rayCount);

    auto programOpt = gl::Program::fromFiles(
        {{"flatland_rc_vert.glsl", gl::Shader::VERTEX},
         {"flatland_rc_frag.glsl", gl::Shader::FRAGMENT}});
    if (!programOpt.has_value()) {
      Logger::error("Failed to load flatland_rc program: {}",
                    programOpt.error());
      return std::nullopt;
    }
    auto& program = programOpt.value();

    gl::Buffer constantsUbo(sizeof(FlatlandRcConstants), nullptr,
                            GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT |
                                GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    constantsUbo.map(GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT |
                     GL_MAP_COHERENT_BIT);

    std::vector<gl::Buffer> paramsUbo;
    paramsUbo.reserve(maxCascades);
    for (uint32_t i = 0; i < maxCascades; i++) {
      gl::Buffer ubo(sizeof(FlatlandRcParams), nullptr,
                     GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT |
                         GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
      ubo.map(GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
      paramsUbo.push_back(std::move(ubo));
    }
    FlipFlops flipFlops(GL_RGBA32F, size, maxCascades);

    gl::Texture resultTex{};
    resultTex.storage(1, GL_RGBA32F, {size.width, size.height});
    gl::Framebuffer resultFbo;
    resultFbo.attachTexture(GL_COLOR_ATTACHMENT0, resultTex);
    TexFbo result{std::move(resultTex), std::move(resultFbo)};

    return FlatlandRc(fullscreenVao, std::move(program), std::move(result),
                      std::move(constantsUbo), std::move(paramsUbo),
                      std::move(flipFlops), rayCount, maxSteps, maxCascades);
  }

  // TODO: Resize

  void draw(const gl::Texture& sceneTexture, const gl::Texture& jfaTexture,
            const glm::vec2& fsize) {
    m_fullscreenVao.bind();
    m_program.bind();

    sceneTexture.bind(0);
    jfaTexture.bind(1);

    constexpr glm::vec2 clear(0.0);

    {
      FlatlandRcConstants params{.resolution = fsize,
                                 .baseRayCount = m_baseRayCount,
                                 .maxSteps = m_maxSteps,
                                 .maxCascades = m_maxCascades};
      void* constMapping = m_constantsUbo.getMapping();
      std::memcpy(constMapping, &params, sizeof(FlatlandRcConstants));
    }
    m_constantsUbo.bindBase(GL_UNIFORM_BUFFER, 0);

    for (int32_t i = m_maxCascades - 1; i >= 0; --i) {
      FlatlandRcParams params{.currentCascade = static_cast<uint32_t>(i)};
      auto mapping = m_paramsUbo[i].getMapping();
      std::memcpy(mapping, &params, sizeof(FlatlandRcParams));
      m_paramsUbo[i].bindBase(GL_UNIFORM_BUFFER, 1);

      if (i >= 1) {
        m_flipFlops[i].fbo.bind();
        glDrawArrays(GL_TRIANGLES, 0, 3);
        m_flipFlops[i].tex.bind(2);
      } else {
        m_result.fbo.bind();
        glDrawArrays(GL_TRIANGLES, 0, 3);
      }
    }
    gl::Framebuffer::unbind();
  }

  void blitToScreen(const gl::Window::Size& size) {
    auto& cascadeFbo =
        m_cascadeIndex == 0 ? m_result.fbo : m_flipFlops[m_cascadeIndex].fbo;
    cascadeFbo.blit(0, 0, 0, size.width, size.height, 0, 0, size.width,
                    size.height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
  }
};