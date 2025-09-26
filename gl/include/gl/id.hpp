#pragma once

#include <glad/glad.h>
#include <spdlog/fmt/bundled/format.h>

namespace gl {
  class Id {
    GLuint m_id = 0;

  public:
    inline explicit Id() = default;
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

template <> struct fmt::formatter<gl::Id> : formatter<GLuint> {
  // parse is inherited from formatter<string_view>.

  auto format(const gl::Id& id, format_context& ctx) const
      -> format_context::iterator {
    return formatter<GLuint>::format(static_cast<GLuint>(id), ctx);
  }
};