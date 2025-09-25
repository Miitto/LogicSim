#include <gl/vao.hpp>

namespace gl {
  void gl::Vao::unbind() { glBindVertexArray(0); }

  void gl::Vao::bindVertexBuffer(GLuint index, const gl::Id& bufferId,
                                 GLuint offset, GLuint stride) {
    glVertexArrayVertexBuffer(m_id, index, bufferId, offset, stride);
  }

  void gl::Vao::attribFormat(GLuint index, GLuint numComponents, GLenum type,
                             bool normalize, GLuint offset,
                             std::optional<GLuint> bufferIndex) {
    glEnableVertexArrayAttrib(m_id, index);
    switch (type) {
    case GL_INT:
    case GL_UNSIGNED_INT: {
      glVertexArrayAttribIFormat(m_id, index, numComponents, type, offset);
      break;
    }
    default: {
      glVertexArrayAttribFormat(m_id, index, numComponents, type,
                                normalize ? GL_TRUE : GL_FALSE, offset);
      break;
    }
    }
    if (bufferIndex.has_value()) {
      glVertexArrayAttribBinding(m_id, index, bufferIndex.value());
    }
  }

  void gl::Vao::bindAttribs(GLuint bufferIndex,
                            std::initializer_list<GLuint> attribIndices) {
    for (const auto& index : attribIndices) {
      glVertexArrayAttribBinding(m_id, index, bufferIndex);
    }
  }
} // namespace gl
