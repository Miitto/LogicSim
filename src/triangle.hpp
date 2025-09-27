#pragma once

#include <array>
#include <gl/gl.hpp>
#include <glm/glm.hpp>
#include <optional>

class Triangle {
  gl::Vao m_vao;
  gl::Buffer m_vbo;
  gl::Program m_program;
  const glm::vec4& m_clearColor;

  Triangle(gl::Vao&& vao, gl::Buffer&& vbo, gl::Program&& program,
           const glm::vec4& clearColor)
      : m_vao(std::move(vao)), m_vbo(std::move(vbo)),
        m_program(std::move(program)), m_clearColor(clearColor) {}

public:
  struct TriVertex {
    glm::vec3 position;
    glm::vec3 color;
  };

  static std::optional<Triangle> create(const glm::vec4& clearColor) {
    auto programOpt =
        gl::Program::fromFiles({{"basic_vert.glsl", gl::Shader::VERTEX},
                                {"basic_frag.glsl", gl::Shader::FRAGMENT}});
    if (!programOpt.has_value()) {
      Logger::error("Failed to load basic program: {}", programOpt.error());
      return std::nullopt;
    }
    auto& program = programOpt.value();
    std::array<TriVertex, 3> vertices = {
        TriVertex{.position = {0.0f, 0.5f, 0.0f}, .color = {1.0f, 0.0f, 0.0f}},
        TriVertex{.position = {0.5f, -0.5f, 0.0f}, .color = {0.0f, 1.0f, 0.0f}},
        TriVertex{.position = {-0.5f, -0.5f, 0.0f},
                  .color = {0.0f, 0.0f, 1.0f}},
    };
    gl::Vao vao{};
    gl::Buffer vbo(3 * sizeof(TriVertex), &vertices);
    vao.bindVertexBuffer(0, vbo.id(), 0, sizeof(TriVertex));
    vao.attribFormat(0, 3, GL_FLOAT, false, offsetof(TriVertex, position), 0);
    vao.attribFormat(1, 3, GL_FLOAT, false, offsetof(TriVertex, color), 0);
    return Triangle(std::move(vao), std::move(vbo), std::move(program),
                    clearColor);
  }

  void draw() {
    glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b,
                 m_clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT);
    m_program.bind();
    m_vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);
  }
};
