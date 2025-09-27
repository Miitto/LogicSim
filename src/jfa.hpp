#pragma once

#include "flipFlops.hpp"
#include <gl/gl.hpp>
#include <glm/glm.hpp>

class Jfa {
  const gl::Vao& m_fullscreenVao;

  struct Programs {
    gl::Program toUv;
    gl::Program jumpFlood;
    gl::Program distance;
  };

  Programs m_programs;

  FlipFlops m_flipFlops;

  std::vector<gl::Buffer> m_ubos;

  struct JfaResult {
    gl::Texture texture;
    gl::Framebuffer fbo;
  };

  JfaResult m_result;

  struct DistanceResult {
    gl::Texture texture;
    gl::Framebuffer fbo;
  };

  DistanceResult m_distanceResult;

  uint32_t m_jfaPasses;
  uint32_t m_maxJfaPasses;

  Jfa(const gl::Vao& fullscreenVao, Programs&& programs, FlipFlops&& flipFlops,
      std::vector<gl::Buffer> ubos, JfaResult&& result,
      DistanceResult&& distanceResult, uint32_t jfaPasses,
      uint32_t maxJfaPasses, const gl::Window& window)
      : m_fullscreenVao(fullscreenVao), m_programs(std::move(programs)),
        m_flipFlops(std::move(flipFlops)), m_ubos(std::move(ubos)),
        m_result(std::move(result)),
        m_distanceResult(std::move(distanceResult)), m_jfaPasses(jfaPasses),
        m_maxJfaPasses(maxJfaPasses) {
    setupUbos(window.size());
  }

  void setupUbos(const gl::Window::Size& size) {
    glm::vec2 fsize{static_cast<float>(size.width),
                    static_cast<float>(size.height)};

    for (size_t i = 0; i < m_ubos.size(); i++) {
      auto* mapping = static_cast<JfaParams*>(m_ubos[i].getMapping());
      mapping->offset =
          glm::vec2(static_cast<float>(pow(2, m_jfaPasses - i - 1))) / fsize;
    }
  }

public:
  struct JfaParams {
    glm::vec2 offset;
  };

  uint32_t& passes() { return m_jfaPasses; }
  uint32_t maxPasses() const { return m_maxJfaPasses; }

  const JfaResult& result() const { return m_result; }
  const DistanceResult& distanceResult() const { return m_distanceResult; }

  static std::optional<Jfa> create(const gl::Vao& fullscreenVao,
                                   const gl::Window& window);

  void resize(const gl::Window::Size& size) {
    m_flipFlops = FlipFlops(GL_RGBA32F, size, 2);

    m_result = JfaResult{};
    m_result.texture.storage(1, GL_RGBA32F, {size.width, size.height});
    m_result.fbo.attachTexture(GL_COLOR_ATTACHMENT0, m_result.texture);

    m_distanceResult = DistanceResult{};
    m_distanceResult.texture.storage(1, GL_R32F, {size.width, size.height});
    m_distanceResult.fbo.attachTexture(GL_COLOR_ATTACHMENT0,
                                       m_distanceResult.texture);

    uint32_t maxJfaPasses =
        static_cast<uint32_t>(ceil(log2(std::max(size.width, size.height))));

    if (maxJfaPasses > m_maxJfaPasses) {
      // Add more UBOs
      for (uint32_t i = m_maxJfaPasses; i < maxJfaPasses; i++) {
        gl::Buffer ubo(sizeof(JfaParams), nullptr,
                       GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT |
                           GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
        ubo.map(GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
        m_ubos.push_back(std::move(ubo));
      }
    } else if (maxJfaPasses < m_maxJfaPasses) {
      // Remove extra UBOs
      m_ubos.resize(maxJfaPasses);
    }

    m_maxJfaPasses = maxJfaPasses;

    if (m_jfaPasses > m_maxJfaPasses) {
      m_jfaPasses = m_maxJfaPasses;
    }

    setupUbos(size);
  }

  void draw(const gl::Texture& drawTexture, gl::Window::Size size,
            glm::vec2 fsize) {
#pragma region ToUV
    m_programs.toUv.bind();
    m_fullscreenVao.bind();
    drawTexture.bind(0);
    m_flipFlops[0].fbo.bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);
#pragma endregion

#pragma region JFA
    if (m_jfaPasses != 0) {
      m_programs.jumpFlood.bind();
      m_fullscreenVao.bind();

      const gl::Texture* inTex = &m_flipFlops[0].tex;
      const gl::Framebuffer* output = &m_flipFlops[1].fbo;

      glm::vec2 oneOverResolution = {1.f / fsize.x, 1.f / fsize.y};

      for (uint32_t i = 0; i < m_jfaPasses; i++) {
        inTex->bind(0);
        output->bind();
        m_ubos[i].bindBase(GL_UNIFORM_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        if (i % 2 == 0) {
          inTex = &m_flipFlops[1].tex;
          output = &m_flipFlops[0].fbo;
        } else {
          inTex = &m_flipFlops[0].tex;
          output = &m_flipFlops[1].fbo;
        }
      }
    }
    gl::Framebuffer::unbind();

    if (m_jfaPasses % 2 == 1 || m_jfaPasses == 0) {
      m_flipFlops[0].fbo.blit(m_result.fbo.id(), 0, 0, size.width, size.height,
                              0, 0, size.width, size.height,
                              GL_COLOR_BUFFER_BIT, GL_LINEAR);
    } else {
      m_flipFlops[1].fbo.blit(m_result.fbo.id(), 0, 0, size.width, size.height,
                              0, 0, size.width, size.height,
                              GL_COLOR_BUFFER_BIT, GL_LINEAR);
    }
#pragma endregion

#pragma region Distance
    m_programs.distance.bind();
    m_fullscreenVao.bind();
    m_result.texture.bind(0);
    m_distanceResult.fbo.bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);
    gl::Framebuffer::unbind();
#pragma endregion
  }

  void blitToMain(const gl::Window::Size& size) {
    m_result.fbo.blit(0, 0, 0, size.width, size.height, 0, 0, size.width,
                      size.height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
  }

  void blitDistanceToMain(const gl::Window::Size& size) {
    m_distanceResult.fbo.blit(0, 0, 0, size.width, size.height, 0, 0,
                              size.width, size.height, GL_COLOR_BUFFER_BIT,
                              GL_LINEAR);
  }
};
