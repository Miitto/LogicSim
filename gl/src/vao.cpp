#include <gl/vao.hpp>

void gl::Vao::unbind() { glBindVertexArray(0); }
