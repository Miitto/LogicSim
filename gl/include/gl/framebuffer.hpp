#pragma once

#include <gl/id.hpp>
#include <gl/texture.hpp>
#include <glad/glad.h>

namespace gl {
  class Framebuffer {
    gl::Id m_id = 0;

  public:
    Framebuffer() { glCreateFramebuffers(1, m_id); }
    const gl::Id& id() const { return m_id; }

    void attachTexture(GLenum attachment, GLuint texture,
                       GLint level = 0) const {
      glNamedFramebufferTexture(m_id, attachment, texture, level);
    }
    void attachTexture(GLenum attachment, const gl::Texture& texture,
                       GLint level = 0) const {
      glNamedFramebufferTexture(m_id, attachment, texture.id(), level);
    }
    void bind(GLenum target = GL_FRAMEBUFFER) const {
      glBindFramebuffer(target, m_id);
    }
    void bindRead() const { glBindFramebuffer(GL_READ_FRAMEBUFFER, m_id); }
    void bindDraw() const { glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_id); }
    static void unbind(GLenum target = GL_FRAMEBUFFER) {
      glBindFramebuffer(target, 0);
    }
    void checkStatus(GLenum target = GL_FRAMEBUFFER) const {
      GLenum status = glCheckNamedFramebufferStatus(m_id, target);
      if (status != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("Framebuffer is not complete");
      }
    }
    void blit(GLuint target, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
              GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
              GLbitfield mask = 0, GLenum filter = 0) const {
      glBlitNamedFramebuffer(m_id, target, srcX0, srcY0, srcX1, srcY1, dstX0,
                             dstY0, dstX1, dstY1, mask, filter);
    }
  };
} // namespace gl