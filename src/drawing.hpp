#pragma once

#include <gl/gl.hpp>
#include <glm/glm.hpp>

class Drawing {
  const gl::Vao& m_fullscreenVao;
  gl::Program m_program;
  gl::Texture m_texture;
  gl::Framebuffer m_fbo;

  gl::StorageBuffer m_ubo;
  void* m_uboMapping;

  float m_brushRadius = 5.f;
  glm::vec3 m_brushColor{1.f, 0.f, 0.f};

  Drawing(const gl::Vao& fullscreenVao, gl::Program&& drawProgram,
          gl::Texture&& drawTexture, gl::Framebuffer&& drawFbo,
          gl::StorageBuffer&& ubo, void* uboMapping)
      : m_fullscreenVao(fullscreenVao), m_program(std::move(drawProgram)),
        m_texture(std::move(drawTexture)), m_fbo(std::move(drawFbo)),
        m_ubo(std::move(ubo)), m_uboMapping(uboMapping) {}

public:
  struct DrawParams {
    glm::vec2 from;
    glm::vec2 to;
    glm::vec4 color;
    glm::vec2 resolution;
  };

  float& brushRadius() { return m_brushRadius; }
  glm::vec3& brushColor() { return m_brushColor; }

  const gl::Framebuffer& fbo() const { return m_fbo; }
  const gl::Texture& texture() const { return m_texture; }

  static std::optional<Drawing> create(const gl::Vao& fullscreenVao,
                                       const gl::Window::Size& size) {
    auto drawProgramOpt =
        gl::Program::fromFiles({{"draw_vert.glsl", gl::Shader::VERTEX},
                                {"draw_frag.glsl", gl::Shader::FRAGMENT}});
    if (!drawProgramOpt.has_value()) {
      Logger::error("Failed to load draw program: {}", drawProgramOpt.error());
      return std::nullopt;
    }
    auto& drawProgram = drawProgramOpt.value();
    gl::Texture drawTexture{};
    drawTexture.storage(1, GL_RGBA32F, {size.width, size.height});
    gl::Framebuffer drawFbo;
    drawFbo.attachTexture(GL_COLOR_ATTACHMENT0, drawTexture);

    gl::StorageBuffer drawParamsBuffer(
        sizeof(DrawParams), nullptr,
        gl::Buffer::Usage::DYNAMIC | gl::Buffer::Usage::WRITE |
            gl::Buffer::Usage::PERSISTENT | gl::Buffer::Usage::COHERENT);

    auto drawMapping = drawParamsBuffer.map(gl::Buffer::Mapping::WRITE |
                                            gl::Buffer::Mapping::PERSISTENT |
                                            gl::Buffer::Mapping::COHERENT);

    return Drawing(fullscreenVao, std::move(drawProgram),
                   std::move(drawTexture), std::move(drawFbo),
                   std::move(drawParamsBuffer), drawMapping);
  }

  void resize(const gl::Window::Size& size) {
    auto framebufferSize = m_texture.size();
    Logger::info("Resizing framebuffer to {}x{}", size.width, size.height);
    gl::Texture newTexture{};
    newTexture.storage(1, GL_RGBA32F, {size.width, size.height});
    gl::Framebuffer newFbo{};
    newFbo.attachTexture(GL_COLOR_ATTACHMENT0, newTexture);

    m_fbo.blit(newFbo.id(), 0, 0, framebufferSize.width, framebufferSize.height,
               0, 0, size.width, size.height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    m_texture = std::move(newTexture);
    m_fbo = std::move(newFbo);
  }

  void draw(const Input& input, const glm::vec2& fsize) {
    if (input.mouse().isButtonDown(0)) {
      DrawParams params{
          .from = input.mouse().lastPosition(),
          .to = input.mouse().position,
          .color = {m_brushColor, m_brushRadius},
          .resolution = fsize,
      };

      memcpy(m_uboMapping, &params, sizeof(DrawParams));
      m_ubo.bindBase(gl::StorageBuffer::Target::UNIFORM, 0);
      m_fbo.bind();
      m_program.bind();
      m_fullscreenVao.bind();
      glDrawArrays(GL_TRIANGLES, 0, 3);
      gl::Framebuffer::unbind();
    }
  }
};
