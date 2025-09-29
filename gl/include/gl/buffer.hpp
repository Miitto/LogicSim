#pragma once

#include <gl/bitflag.hpp>
#include <gl/id.hpp>
#include <glad/glad.h>
#include <limits>

namespace gl {
  class Buffer {
  protected:
    gl::Id m_id = 0;
    GLuint m_size = 0;
    // Topmost bit is set when persistently mapped
    void* m_mapping = nullptr;

  public:
    enum class Usage {
      DEFAULT = 0,
      READ = GL_MAP_READ_BIT,
      WRITE = GL_MAP_WRITE_BIT,
      DYNAMIC = GL_DYNAMIC_STORAGE_BIT,
      PERSISTENT = GL_MAP_PERSISTENT_BIT,
      COHERENT = GL_MAP_COHERENT_BIT,
      FLUSH_EXPLICIT = GL_MAP_FLUSH_EXPLICIT_BIT
    };
    using UsageBitFlag = Bitflag<Usage>;

    enum class Mapping {
      READ = GL_MAP_READ_BIT,
      WRITE = GL_MAP_WRITE_BIT,
      PERSISTENT = GL_MAP_PERSISTENT_BIT,
      COHERENT = GL_MAP_COHERENT_BIT,
      INVALIDATE_BUFFER = GL_MAP_INVALIDATE_BUFFER_BIT,
      INVALIDATE_RANGE = GL_MAP_INVALIDATE_RANGE_BIT,
      FLUSH_EXPLICIT = GL_MAP_FLUSH_EXPLICIT_BIT,
      UNSYNCHRONIZED = GL_MAP_UNSYNCHRONIZED_BIT
    };
    using MappingBitFlag = Bitflag<Mapping>;

    inline ~Buffer() {
      if (m_id != 0)
        glDeleteBuffers(1, m_id);
    }

    inline void init(GLuint size, const void* data = nullptr,
                     UsageBitFlag usage = Usage::DEFAULT);

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    Buffer(Buffer&& other) noexcept = default;
    Buffer& operator=(Buffer&& other) noexcept = default;

    inline const gl::Id& id() const { return m_id; }
    static void unbind(GLenum target);

    void* map(MappingBitFlag flags, GLuint offset = 0,
              GLuint length = std::numeric_limits<GLuint>::max());
    void unmap();

    void* getMapping() const;

  protected:
    inline Buffer() { glCreateBuffers(1, m_id); }

    inline Buffer(GLuint size, const void* data = nullptr,
                  UsageBitFlag usage = Usage::DEFAULT)
        : Buffer() {
      m_size = size;
      glNamedBufferStorage(m_id, size, data, usage);
    }
  };

  class BasicBuffer : public Buffer {
  public:
    enum class Target {
      ARRAY = GL_ARRAY_BUFFER,
      ATOMIC_COUNTER = GL_ATOMIC_COUNTER_BUFFER,
      COPY_READ = GL_COPY_READ_BUFFER,
      COPY_WRITE = GL_COPY_WRITE_BUFFER,
      DISPATCH_INDIRECT = GL_DISPATCH_INDIRECT_BUFFER,
      DRAW_INDIRECT = GL_DRAW_INDIRECT_BUFFER,
      ELEMENT_ARRAY = GL_ELEMENT_ARRAY_BUFFER,
      PIXEL_PACK = GL_PIXEL_PACK_BUFFER,
      PIXEL_UNPACK = GL_PIXEL_UNPACK_BUFFER,
      QUERY = GL_QUERY_BUFFER,
      SHADER_STORAGE = GL_SHADER_STORAGE_BUFFER,
      TEXTURE = GL_TEXTURE_BUFFER,
      TRANSFORM_FEEDBACK = GL_TRANSFORM_FEEDBACK_BUFFER,
      UNIFORM = GL_UNIFORM_BUFFER
    };

    using TargetBitFlag = Bitflag<Target>;

    BasicBuffer() : Buffer() {}
    BasicBuffer(GLuint size, const void* data = nullptr,
                UsageBitFlag usage = Usage::DEFAULT)
        : Buffer(size, data, usage) {}
    ~BasicBuffer() {}
    BasicBuffer(BasicBuffer&&) = default;
    BasicBuffer& operator=(BasicBuffer&&) = default;

    inline void bind(TargetBitFlag target) const { glBindBuffer(target, m_id); }
  };

  class StorageBuffer : public Buffer {
  public:
    StorageBuffer() : Buffer() {}
    StorageBuffer(GLuint size, const void* data = nullptr,
                  UsageBitFlag usage = Usage::DEFAULT)
        : Buffer(size, data, usage) {}
    ~StorageBuffer() {}
    StorageBuffer(StorageBuffer&&) = default;
    StorageBuffer& operator=(StorageBuffer&&) = default;

    enum class Target {
      UNIFORM = GL_UNIFORM_BUFFER,
      STORAGE = GL_SHADER_STORAGE_BUFFER
    };

    using TargetBitFlag = Bitflag<Target>;

    inline void bindBase(TargetBitFlag target, GLuint index) const {
      glBindBufferBase(target, index, m_id);
    }
    inline void bindRange(TargetBitFlag target, GLuint index, GLuint offset,
                          GLuint size) const {
      glBindBufferRange(target, index, m_id, offset, size);
    }
  };
} // namespace gl