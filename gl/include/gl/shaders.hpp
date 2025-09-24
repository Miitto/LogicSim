#pragma once

#include <gl/id.hpp>
#include <glad/glad.h>
#include <optional>
#include <span>
#include <string_view>

namespace gl {
  class Shader {
    gl::Id m_id;

  public:
    enum Type { VERTEX = GL_VERTEX_SHADER, FRAGMENT = GL_FRAGMENT_SHADER };
    Shader(Type type, std::string_view source);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&&) noexcept = default;
    Shader& operator=(Shader&&) noexcept = default;

    const gl::Id& id() const { return m_id; }

    static std::optional<Shader> fromFile(std::string_view path, Type type);
  };

  class Program {
    gl::Id m_id = 0;

    void handleLinkFail();

  public:
    template <typename... Shaders> inline Program(Shaders&... shaders) {
      m_id = glCreateProgram();
      (glAttachShader(m_id, shaders.id()), ...);
      glLinkProgram(m_id);
    }

    inline Program(std::span<std::reference_wrapper<Shader>> shaders) {
      m_id = glCreateProgram();
      for (const auto& shader : shaders) {
        glAttachShader(m_id, shader.get().id());
      }
      glLinkProgram(m_id);
    }

    void bind() const { glUseProgram(m_id); }
  };
} // namespace gl