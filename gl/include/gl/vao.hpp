#pragma once

#include <gl/id.hpp>
#include <glad/glad.h>
#include <optional>

namespace gl {

  class Vao {
    gl::Id m_id = 0;

  public:
    inline Vao() { glCreateVertexArrays(1, m_id); }
    inline ~Vao() {
      if (m_id != 0)
        glDeleteVertexArrays(1, m_id);
    }
    Vao(const Vao&) = delete;
    Vao& operator=(const Vao&) = delete;
    Vao(Vao&& other) noexcept = default;
    Vao& operator=(Vao&& other) noexcept = default;

    inline const gl::Id& id() const { return m_id; }
    inline void bind() const { glBindVertexArray(m_id); }
    static void unbind();

    /// <summary>
    /// Bind a vertex buffer to this VAO.
    /// Uses DSA
    ///</summary>
    /// <param name="index"></param> Index to bind the buffer to
    /// <param name="bufferId"></param> Id of the buffer to bind
    /// <param name="offset"></param> Offset in the buffer to start from
    /// <param name="stride"></param> Stride between vertices
    void bindVertexBuffer(GLuint index, const gl::Id& bufferId, GLuint offset,
                          GLuint stride);
    /// <summary>
    /// Set the format of a vertex attribute. Will also enable the attribute.
    /// Uses DSA
    /// </summary>
    /// <param name="index"></param> Index of the attribute to set
    /// <param name="numComponents"></param>
    /// <param name="type"></param>
    /// <param name="normalize"></param>
    /// <param name="offset"></param>
    /// <param name="bufferIndex"></param> Optional index of the buffer to bind
    /// this attribute to
    void attribFormat(GLuint index, GLuint numComponents, GLenum type,
                      bool normalize, GLuint offset,
                      std::optional<GLuint> bufferIndex = std::nullopt);

    void bindAttribs(GLuint bufferIndex,
                     std::initializer_list<GLuint> attribIndices);
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