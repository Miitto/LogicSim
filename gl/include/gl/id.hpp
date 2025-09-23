#pragma once

#include <glad/glad.h>

namespace gl {
  class Id {
    GLuint m_id = 0;

  public:
    inline Id(GLuint id) : m_id(id) {}
    ~Id() = default;

    Id(const Id&) = delete;
    Id& operator=(const Id&) = delete;

    Id(Id&& other) noexcept : m_id(other.m_id) { other.m_id = 0; }
    Id& operator=(Id&& other) noexcept {
      if (this != &other) {
        m_id = other.m_id;
        other.m_id = 0;
      }
      return *this;
    }
    Id& operator=(GLuint id) {
      m_id = id;
      return *this;
    }

    inline operator GLuint() const { return m_id; }
    inline operator GLuint*() { return &m_id; }
  };
} // namespace gl