#pragma once
#include <glm/glm.hpp>

namespace renderer {
  class Camera {
    glm::vec2 m_position;
    glm::vec2 m_scale;

  public:
    Camera(const glm::vec2& position, float zoom,
           const glm::vec2& window_dimensions);

    void translate(const glm::vec2& delta);
    void zoom(float factor);
    void zoomTo(float zoom);

    void onWindowResize(const glm::vec2& window_dimensions);
  };
} // namespace renderer