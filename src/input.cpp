#include "input.hpp"
#include "logger.hpp"

void Input::glfwKeyCallback(GLFWwindow* window, int key, int scancode,
                            int action, int mods) {
  Input* input = reinterpret_cast<Input*>(gl::Window::getUserPtr(window));
  input->onKeyEvent(key, action);

  (void)scancode;
  (void)mods;
}

void Input::glfwCursorPosCallback(GLFWwindow* window, double xpos,
                                  double ypos) {
  Input* input = reinterpret_cast<Input*>(gl::Window::getUserPtr(window));
  input->onMouseMove(xpos, ypos);
}

void Input::glfwMouseButtonCallback(GLFWwindow* window, int button, int action,
                                    int mods) {
  Input* input = reinterpret_cast<Input*>(gl::Window::getUserPtr(window));
  (void)mods;
  input->onMouseButton(button, action);
}

void Input::onKeyEvent(int key, int action) {
  if (m_imguiWantsKeyboard && action != GLFW_RELEASE) {
    return;
  }
  KeyState state = action == GLFW_PRESS    ? KeyState::Pressed
                   : action == GLFW_REPEAT ? KeyState::PressedRepeat
                                           : KeyState::Released;
  keyState[key] = state;
  Logger::debug("Key {} is now {}", key, state);
}
void Input::onMouseMove(double x, double y) {
  if (m_imguiWantsMouse) {
    // If ImGui wants the mouse, don't update delta (still want position for
    // accurate tracking)
    m_mouse.position = {x, y};
    return;
  }
  m_mouse.delta += glm::vec2{x - m_mouse.position.x, y - m_mouse.position.y};
  m_mouse.position = {x, y};
}

void Input::onMouseButton(int button, int action) {
  if (m_imguiWantsMouse && action != GLFW_RELEASE) {
    return;
  }

  Logger::debug("Clicked at {}, {}", m_mouse.position.x, m_mouse.position.y);

  switch (action) {
  case GLFW_PRESS:
    m_mouse.onClick(button);
    break;
  case GLFW_RELEASE:
    m_mouse.onRelease(button);
    break;
  }
}

void Input::frameEnd() {
  m_mouse.delta = {0, 0};
  std::vector<int> keys;
  keys.reserve(keyState.size());
  for (auto [key, state] : keyState) {
    if (state == KeyState::Pressed) {
      state = KeyState::PressedRepeat;
    } else if (state == KeyState::Released) {
      keys.push_back(key);
    }
  }

  for (auto& key : keys) {
    keyState.erase(key);
  }
}

auto fmt::formatter<KeyState>::format(KeyState ks, format_context& ctx) const
    -> format_context::iterator {
  string_view str = "";
  switch (ks) {
  case KeyState::Pressed:
    str = "Pressed";
    break;
  case KeyState::PressedRepeat:
    str = "PressedRepeat";
    break;
  case KeyState::Released:
    str = "Released";
    break;
  }
  return formatter<string_view>::format(str, ctx);
}