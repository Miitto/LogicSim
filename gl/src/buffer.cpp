#include "logger.hpp"
#include <gl/buffer.hpp>

namespace {
  constexpr intptr_t PERSISTENT_MAP_BIT = 1LL << 63L;

  inline void* setPersistentBit(void* ptr) {
    intptr_t iptr = reinterpret_cast<intptr_t>(ptr);
    iptr |= PERSISTENT_MAP_BIT;
    return reinterpret_cast<void*>(iptr);
  }

  inline void* clearPersistentBit(void* ptr) {
    intptr_t iptr = reinterpret_cast<intptr_t>(ptr);
    iptr &= ~PERSISTENT_MAP_BIT;
    return reinterpret_cast<void*>(iptr);
  }
} // namespace

namespace gl {
  inline void gl::Buffer::init(GLuint size, const void* data,
                               UsageBitFlag flags) {
#ifndef NDEBUG
    if (m_size != 0) {
      gl::Logger::error("Attempted to reinitialize a buffer");
      return;
    }
#endif
    m_size = size;
    glNamedBufferStorage(m_id, size, data, flags);
  }

  void* gl::Buffer::map(MappingBitFlag flags, GLuint offset, GLuint length) {
    length = length == std::numeric_limits<GLuint>::max() ? m_size : length;

    m_mapping = glMapNamedBufferRange(m_id, offset, length, flags);
    if (flags & GL_MAP_PERSISTENT_BIT) {
      m_mapping = setPersistentBit(m_mapping);
    }
    return clearPersistentBit(m_mapping);
  }

  inline void gl::Buffer::unmap() {
    if (m_mapping) {
#ifndef NDEBUG
      if ((reinterpret_cast<intptr_t>(m_mapping) & PERSISTENT_MAP_BIT) != 0) {
        gl::Logger::warn("Unmapped a persistently mapped buffer");
      }
#endif
      glUnmapNamedBuffer(m_id);
      m_mapping = nullptr;
    }
  }

  void* gl::Buffer::getMapping() const { return clearPersistentBit(m_mapping); }

  void gl::Buffer::unbind(GLenum target) { glBindBuffer(target, 0); }
} // namespace gl
