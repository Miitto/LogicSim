#include "gl/buffer.hpp"
#include "logger.hpp"

inline void gl::Buffer::init(GLuint size, const void* data, GLbitfield flags) {
#ifndef NDEBUG
  if (m_size != 0) {
    gl::Logger::error("Attempted to reinitialize a buffer");
    return;
  }
#endif
  m_size = size;
  glNamedBufferStorage(m_id, size, data, flags);
}

inline void* gl::Buffer::map(GLbitfield flags, GLuint offset, GLuint length) {
  length = length == std::numeric_limits<GLuint>::max() ? m_size : length;

  m_mapping = glMapNamedBufferRange(m_id, offset, length, flags);
  if (flags & GL_MAP_PERSISTENT_BIT) {
    void** mapping = &m_mapping;
    intptr_t* ptr = reinterpret_cast<intptr_t*>(mapping);
    (*ptr) |= (1LL << 63L);
  }
  return m_mapping;
}

inline void gl::Buffer::unmap() {

  if (m_mapping) {
#ifndef NDEBUG
    if (((reinterpret_cast<intptr_t>(m_mapping) >> 63) & 1) == 1) {
      gl::Logger::warn("Unmapped a persistently mapped buffer");
    }
#endif
    glUnmapNamedBuffer(m_id);
    m_mapping = nullptr;
  }
}

void gl::Buffer::unbind(GLenum target) { glBindBuffer(target, 0); }
