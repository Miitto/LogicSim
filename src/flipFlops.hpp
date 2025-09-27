#pragma once

#include <gl/gl.hpp>

struct TexFbo {
  gl::Texture tex;
  gl::Framebuffer fbo;
};

template <size_t num> class FlipFlops {
  std::array<TexFbo, num> buffers;

public:
  FlipFlops(TexFbo&& items...) : buffers(std::forward(items...)) {}
  FlipFlops(GLenum internalFormat, const gl::Window::Size& size) {
    for (size_t i = 0; i < num; i++) {
      buffers[i].tex.storage(1, internalFormat, {size.width, size.height});
      buffers[i].fbo.attachTexture(GL_COLOR_ATTACHMENT0, buffers[i].tex);
    }
  }

  const TexFbo& operator[](size_t index) const { return buffers[index]; }
};
