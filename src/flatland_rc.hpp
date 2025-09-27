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

  std::vector<gl::Buffer> m_paramsUbo;

  FlipFlops<2> m_flipFlops;

  const uint32_t& m_baseRayCount;
  const uint32_t& m_maxSteps;

  uint32_t m_cascadeIndex = 0;

  FlatlandRc(const gl::Vao& fullscreenVao, gl::Program&& rcProgram,
             TexFbo&& result, std::vector<gl::Buffer>&& paramsUbo,
             FlipFlops<2>&& flipFlops, const uint32_t& rayCount,
             const uint32_t& maxSteps)
      : m_fullscreenVao(fullscreenVao), m_program(std::move(rcProgram)),
        m_result(std::move(result)), m_paramsUbo(std::move(paramsUbo)),
        m_flipFlops(std::move(flipFlops)), m_baseRayCount(rayCount),
        m_maxSteps(maxSteps) {}

public:
  struct FlatlandRcParams {
    glm::vec2 resolution;
    uint32_t rayCount;
    uint32_t baseRayCount;
    uint32_t maxSteps;
  };

  const uint32_t& cascadeIndex() const { return m_cascadeIndex; }

  static std::optional<FlatlandRc> create(const gl::Vao& fullscreenVao,
                                          const uint32_t& rayCount,
                                          const uint32_t& maxSteps,
                                          const gl::Window::Size& size) {
    auto programOpt = gl::Program::fromFiles(
        {{"flatland_rc_vert.glsl", gl::Shader::VERTEX},
         {"flatland_rc_frag.glsl", gl::Shader::FRAGMENT}});
    if (!programOpt.has_value()) {
      Logger::error("Failed to load flatland_rc program: {}",
                    programOpt.error());
      return std::nullopt;
    }
    auto& program = programOpt.value();

    std::vector<gl::Buffer> paramsUbo;
    paramsUbo.reserve(2);
    for (int i = 0; i < 2; i++) {
      gl::Buffer ubo(sizeof(FlatlandRcParams), nullptr,
                     GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT |
                         GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
      ubo.map(GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
      paramsUbo.push_back(std::move(ubo));
    }
    FlipFlops<2> flipFlops(GL_RGBA8, size);

    gl::Texture resultTex{};
    resultTex.storage(1, GL_RGBA8, {size.width, size.height});
    gl::Framebuffer resultFbo;
    resultFbo.attachTexture(GL_COLOR_ATTACHMENT0, resultTex);
    TexFbo result{std::move(resultTex), std::move(resultFbo)};

    return FlatlandRc(fullscreenVao, std::move(program), std::move(result),
                      std::move(paramsUbo), std::move(flipFlops), rayCount,
                      maxSteps);
  }

  void draw(const gl::Texture& sceneTexture, const gl::Texture& jfaTexture,
            const glm::vec2& fsize) {
    m_fullscreenVao.bind();
    m_program.bind();

    sceneTexture.bind(0);
    jfaTexture.bind(1);

    constexpr glm::vec2 clear(0.0);

    for (int i = 2; i >= 1; --i) {
      uint32_t rayCount = static_cast<uint32_t>(pow(m_baseRayCount, i));
      FlatlandRcParams params{
          .resolution = fsize,
          .rayCount = rayCount,
          .baseRayCount = m_baseRayCount,
          .maxSteps = m_maxSteps,
      };
      auto mapping = m_paramsUbo[i - 1].getMapping();
      std::memcpy(mapping, &params, sizeof(FlatlandRcParams));
      m_paramsUbo[i - 1].bindBase(GL_UNIFORM_BUFFER, 0);

      if (i > 1) {
        m_flipFlops[i - 1].fbo.bind();
        glDrawArrays(GL_TRIANGLES, 0, 3);
        m_flipFlops[i - 1].tex.bind(2);
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
                    size.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
  }
};