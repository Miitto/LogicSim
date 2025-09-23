#pragma once

#include <gl/id.hpp>
#include <glad/glad.h>
#include <span>
#include <string_view>

namespace gl {
  class Shader {
    gl::Id m_id;

  public:
    enum Type { VERTEX = GL_VERTEX_SHADER, FRAGMENT = GL_FRAGMENT_SHADER };
    Shader(Type type, std::string_view source);
    ~Shader();

    const gl::Id& id() const { return m_id; }
  };

  class Program {
    gl::Id m_id = 0;

    void handleLinkFail();

  public:
    template <typename... Shaders> inline Program(Shaders&... shaders) {
      m_id = glCreateProgram();
      (glAttachShader(m_id, shaders.id()), ...);
      glLinkProgram(m_id);
      GLint success;
      glGetProgramiv(m_id, GL_LINK_STATUS, &success);
      if (!success)
        handleLinkFail();
    }

    inline Program(std::span<std::reference_wrapper<Shader>> shaders) {
      m_id = glCreateProgram();
      for (const auto& shader : shaders) {
        glAttachShader(m_id, shader.get().id());
      }
      glLinkProgram(m_id);
      GLint success;
      glGetProgramiv(m_id, GL_LINK_STATUS, &success);
      if (!success)
        handleLinkFail();
    }

    void bind() const { glUseProgram(m_id); }
  };
} // namespace gl