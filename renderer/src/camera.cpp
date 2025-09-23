#include "renderer/camera.hpp"

namespace renderer {
  renderer::Camera::Camera(const glm::vec2& position, float zoom,
                           const glm::vec2& window_dimensions)
      : m_position(position), m_scale({}) {
    const auto aspect_ratio = window_dimensions.x / window_dimensions.y;

    m_scale.y = zoom;
    m_scale.x = zoom * aspect_ratio;
  }

  void Camera::translate(const glm::vec2& delta) { m_position += delta; }

  void Camera::zoom(float factor) { m_scale *= factor; }

  void Camera::zoomTo(float zoom) {
    const auto aspect_ratio = m_scale.x / m_scale.y;
    m_scale.y = zoom;
    m_scale.x = zoom * aspect_ratio;
  }

  void Camera::onWindowResize(const glm::vec2& window_dimensions) {
    const auto aspect_ratio = window_dimensions.x / window_dimensions.y;
    m_scale.x = m_scale.y * aspect_ratio;
  }
} // namespace renderer