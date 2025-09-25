#pragma once

#include <gl/id.hpp>
#include <glad/glad.h>
#include <limits>

namespace gl {
  class Buffer {
    gl::Id m_id = 0;
    GLuint m_size = 0;
    // Topmost bit is set when persistently mapped
    void* m_mapping = nullptr;

  public:
    inline Buffer() { glCreateBuffers(1, m_id); }
    inline ~Buffer() {
      if (m_id != 0)
        glDeleteBuffers(1, m_id);
    }

    inline Buffer(GLuint size, const void* data = nullptr, GLbitfield flags = 0)
        : Buffer() {
      m_size = size;
      glNamedBufferStorage(m_id, size, data, flags);
    }

    inline void init(GLuint size, const void* data = nullptr,
                     GLbitfield flags = 0);

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    Buffer(Buffer&& other) noexcept = default;
    Buffer& operator=(Buffer&& other) noexcept = default;

    inline const gl::Id& id() const { return m_id; }
    inline void bind(GLenum target) const { glBindBuffer(target, m_id); }
    inline void bindBase(GLenum target, GLuint index) const {
      glBindBufferBase(target, index, m_id);
    }
    inline void bindRange(GLenum target, GLuint index, GLuint offset,
                          GLuint size) const {
      glBindBufferRange(target, index, m_id, offset, size);
    }
    static void unbind(GLenum target);

    void* map(GLbitfield flags, GLuint offset = 0,
              GLuint length = std::numeric_limits<GLuint>::max());
    void unmap();

    void* getMapping() const;
  };
} // namespace gl