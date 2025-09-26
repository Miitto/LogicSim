#pragma once

#include <gl/id.hpp>
#include <glad/glad.h>

namespace gl {
  class Texture {
  public:
    struct Size {
      GLsizei width;
      GLsizei height;
    };

  private:
    gl::Id m_id = 0;
    gl::Texture::Size m_size{};

  public:
    Texture() { glCreateTextures(GL_TEXTURE_2D, 1, m_id); }
    const gl::Id& id() const { return m_id; }

    void bind(GLenum unit) const { glBindTextureUnit(unit, m_id); }
    static void unbind(GLenum unit) { glBindTextureUnit(unit, 0); }
    void setParameter(GLenum pname, GLint param) const {
      glTextureParameteri(m_id, pname, param);
    }
    void storage(GLint level, GLenum internalformat, gl::Texture::Size size) {
      m_size = size;
      glTextureStorage2D(m_id, level, internalformat, size.width, size.height);
    }
    void subImage(GLint level, GLint xoffset, GLint yoffset, GLsizei width,
                  GLsizei height, GLenum format, GLenum type,
                  const void* pixels) const {
      glTextureSubImage2D(m_id, level, xoffset, yoffset, width, height, format,
                          type, pixels);
    }

    const Size& size() const { return m_size; }
  };
} // namespace gl