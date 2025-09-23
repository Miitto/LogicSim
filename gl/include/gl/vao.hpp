#pragma once

#include <gl/id.hpp>
#include <glad/glad.h>

namespace gl {

  class Vao {
    gl::Id m_id = 0;

  public:
    inline Vao() { glCreateVertexArrays(1, m_id); }
    inline ~Vao() {
      if (id != 0)
        glDeleteVertexArrays(1, m_id);
    }
    Vao(const Vao&) = delete;
    Vao& operator=(const Vao&) = delete;
    Vao(Vao&& other) noexcept = default;
    Vao& operator=(Vao&& other) noexcept = default;

    inline const GLuint& id() const { return m_id; }
    inline void bind() const { glBindVertexArray(m_id); }
    static void unbind();

    class BindGuard {
    public:
      ~BindGuard() { Vao::unbind(); }
    };

    inline BindGuard bindGuard() {
      bind();
      return BindGuard();
    }
  };
  ;
} // namespace gl