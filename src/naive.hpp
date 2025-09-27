#pragma once

#include <gl/gl.hpp>
#include <glm/glm.hpp>
#include <optional>

class NaiveRaymarch {
  const gl::Vao& m_fullscreenVao;
  gl::Program m_program;
  gl::Buffer m_paramsUbo;
  void* m_paramsMapping;

  const uint32_t& m_rayCount;
  const uint32_t& m_maxSteps;

  NaiveRaymarch(const gl::Vao& fullscreenVao, gl::Program&& naiveProgram,
                gl::Buffer&& paramsUbo, void* paramsMapping,
                const uint32_t& rayCount, const uint32_t& maxSteps)
      : m_fullscreenVao(fullscreenVao), m_program(std::move(naiveProgram)),
        m_paramsUbo(std::move(paramsUbo)), m_paramsMapping(paramsMapping),
        m_rayCount(rayCount), m_maxSteps(maxSteps) {}

public:
  struct NaiveParams {
    glm::vec2 resolution;
    uint32_t rayCount;
    uint32_t maxSteps;
  };

  const gl::Program& program() const { return m_program; }

  static std::optional<NaiveRaymarch> create(const gl::Vao& fullscreenVao,
                                             const uint32_t& rayCount,
                                             const uint32_t& maxSteps) {
    auto naiveProgramOpt =
        gl::Program::fromFiles({{"naive_vert.glsl", gl::Shader::VERTEX},
                                {"naive_frag.glsl", gl::Shader::FRAGMENT}});
    if (!naiveProgramOpt.has_value()) {
      Logger::error("Failed to load naive program: {}",
                    naiveProgramOpt.error());
      return std::nullopt;
    }
    auto& naiveProgram = naiveProgramOpt.value();

    gl::Buffer paramsUbo(sizeof(NaiveParams), nullptr,
                         GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT |
                             GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

    auto paramsMapping = paramsUbo.map(
        GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    return NaiveRaymarch(fullscreenVao, std::move(naiveProgram),
                         std::move(paramsUbo), paramsMapping, rayCount,
                         maxSteps);
  }

  void draw(const gl::Texture& drawTexture, const gl::Texture& jfaTexture,
            glm::vec2 fsize) {
    drawTexture.bind(0);
    jfaTexture.bind(1);
    m_fullscreenVao.bind();
    m_program.bind();

    NaiveParams nparams{
        .resolution = {fsize.x, fsize.y},
        .rayCount = m_rayCount,
        .maxSteps = m_maxSteps,
    };

    memcpy(m_paramsMapping, &nparams, sizeof(NaiveParams));
    m_paramsUbo.bindBase(GL_UNIFORM_BUFFER, 0);

    glDrawArrays(GL_TRIANGLES, 0, 3);
  }
};