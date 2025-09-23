#include "logger.hpp"
#include <gl/shaders.hpp>

namespace gl {
  Shader::Shader(Type type, std::string_view source) {
    m_id = glCreateShader(type);
    const char* src = source.data();
    glShaderSource(m_id, 1, &src, nullptr);
    glCompileShader(m_id);
    GLint success;
    glGetShaderiv(m_id, GL_COMPILE_STATUS, &success);
    if (!success) {
      char infoLog[512];
      glGetShaderInfoLog(m_id, 512, nullptr, infoLog);
      gl::Logger::error("Shader compilation failed: {}", infoLog);
      glDeleteShader(m_id);
      m_id = 0;
    }
  }
  Shader::~Shader() {
    if (m_id != 0) {
      glDeleteShader(m_id);
    }
  }

  void Program::handleLinkFail() {
    char infoLog[512];
    glGetProgramInfoLog(m_id, 512, nullptr, infoLog);
    gl::Logger::error("Program linking failed: {}", infoLog);
    glDeleteProgram(m_id);
    m_id = 0;
  }

} // namespace gl