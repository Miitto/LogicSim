#pragma once

#include <expected>
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

    static bool handleLinkFail(const gl::Id&& id);

    Program(gl::Id&& id) : m_id(std::move(id)) {}

  public:
    template <typename... Shaders>
      requires(std::is_same<Shaders, Shader>::value && ...)
    inline static std::optional<Program> create(Shaders&... shaders) {
      GLuint id = glCreateProgram();
      (glAttachShader(id, shaders.id()), ...);
      glLinkProgram(id);
      if (handleLinkFail(id)) {

        return std::nullopt;
      }
      return Program(gl::Id(id));
    }

    inline static std::optional<Program> create(std::span<Shader>&& shaders) {
      GLuint id = glCreateProgram();
      for (const auto& shader : shaders) {
        glAttachShader(id, shader.id());
      }
      glLinkProgram(id);
      if (handleLinkFail(id)) {
        return std::nullopt;
      }
      return Program(gl::Id(id));
    }

    void bind() const { glUseProgram(m_id); }
    static std::expected<Program, std::string>
    fromFiles(std::initializer_list<std::pair<std::string_view, Shader::Type>>
                  paths) {
      std::vector<Shader> shaders;
      shaders.reserve(paths.size());
      for (const auto& [path, type] : paths) {
        auto shaderOpt = Shader::fromFile(path, type);
        if (!shaderOpt.has_value()) {
          return std::unexpected(std::string("Failed to load shader from ") +
                                 std::string(path));
        }
        shaders.push_back(std::move(shaderOpt.value()));
      }

      auto programOpt = create(shaders);
      if (!programOpt.has_value()) {
        return std::unexpected("Failed to link program");
      }
      return std::move(programOpt.value());
    }
  };
} // namespace gl