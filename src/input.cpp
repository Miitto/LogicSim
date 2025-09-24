#include "input.hpp"
#include "logger.hpp"

void Input::glfwKeyCallback(GLFWwindow* window, int key, int scancode,
                            int action, int mods) {
  Input* input = reinterpret_cast<Input*>(gl::Window::getUserPtr(window));
  input->onKeyEvent(key, action);

  (void)scancode;
  (void)mods;
}

void Input::onKeyEvent(int key, int action) {
  if (m_imguiWantsKeyboard) {
    return;
  }
  KeyState state = action == GLFW_PRESS    ? KeyState::Pressed
                   : action == GLFW_REPEAT ? KeyState::PressedRepeat
                                           : KeyState::Released;
  keyState[key] = state;
  Logger::debug("Key {} is now {}", key, state);
}
void Input::onMouseMove(int x, int y) {
  if (m_imguiWantsMouse) {
    return;
  }
  Logger::debug("Mouse moved to {}, {}", x, y);
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