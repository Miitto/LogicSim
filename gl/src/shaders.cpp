#include "logger.hpp"
#include <filesystem>
#include <fstream>
#include <gl/shaders.hpp>

namespace gl {
  std::optional<Shader> Shader::fromFile(std::string_view path,
                                         Shader::Type type) {
    constexpr std::size_t readSize = 4096;

    std::string pathStr("./shaders/");
    pathStr.append(path);

    auto absPath = std::filesystem::absolute(pathStr);

    std::ifstream shaderFile(absPath);

    if (!shaderFile.is_open()) {
      gl::Logger::error("Failed to open shader file: {}", absPath.string());
      return std::nullopt;
    }

    std::string source{};
    std::string buffer(readSize, '\0');
    while (shaderFile.read(&buffer[0], readSize)) {
      source.append(buffer, 0, shaderFile.gcount());
    }
    source.append(buffer, 0, shaderFile.gcount());

    shaderFile.close();

    Shader shader(type, source);
    Logger::debug("Compiled shader {}", path);

    return shader;
  }

  Shader::Shader(Type type, std::string_view source) : m_id(gl::Id()) {
    m_id = glCreateShader(type);
    const char* src = source.data();
    glShaderSource(m_id, 1, &src, nullptr);
    glCompileShader(m_id);
  }

  Shader::~Shader() {
    if (m_id != 0) {
      glDeleteShader(m_id);
    }
  }

  bool Program::handleLinkFail(const gl::Id&& id) {
    GLint success;
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
      GLint logLength;
      glGetProgramiv(id, GL_INFO_LOG_LENGTH, &logLength);
      if (logLength <= 0)
        return false;
      std::string infoLog(logLength, '\0');
      glGetProgramInfoLog(id, logLength, nullptr, infoLog.data());
      gl::Logger::error("Program linking failed: {}", infoLog);
      glDeleteProgram(id);
      return true;
    }

    return false;
  }
} // namespace gl