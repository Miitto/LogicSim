#pragma once

#include <gl/window.hpp>
#include <glm/glm.hpp>
#include <spdlog/fmt/bundled/format.h>
#include <unordered_map>

struct Mouse {
  glm::ivec2 position;
  glm::ivec2 delta;
};

enum class KeyState { Pressed, PressedRepeat, Released };

class Input {
  gl::Window& m_window;
  Mouse m_mouse{};
  std::unordered_map<int, KeyState> keyState{};

  bool m_imguiWantsKeyboard = false;
  bool m_imguiWantsMouse = false;

public:
  Input(gl::Window& window) noexcept : m_window(window) {}
  ~Input() = default;

  Input(const Input&) = delete;
  Input& operator=(const Input&) = delete;

  Input(Input&& other) noexcept : m_window(other.m_window) {
    m_window.setUserPtr(this);
  }

  void onKeyEvent(int key, int action);
  void onMouseMove(int x, int y);

  void imGuiWantsKeyboard(bool wants) { m_imguiWantsKeyboard = wants; }
  void imGuiWantsMouse(bool wants) { m_imguiWantsMouse = wants; }

  static void glfwKeyCallback(GLFWwindow* window, int key, int scancode,
                              int action, int mods);

  KeyState getKeyState(int key) const {
    auto it = keyState.find(key);
    return it == keyState.end() ? KeyState::Released : it->second;
  }
  bool isKeyDown(int key) const {
    auto it = keyState.find(key);
    return it != keyState.end() && (it->second == KeyState::Pressed ||
                                    it->second == KeyState::PressedRepeat);
  }
  bool isKeyUp(int key) const {
    auto it = keyState.find(key);
    return it == keyState.end() || it->second == KeyState::Released;
  }
  bool isKeyPressed(int key) const {
    auto it = keyState.find(key);
    return it != keyState.end() && it->second == KeyState::Pressed;
  }

  const Mouse& mouse() const { return m_mouse; }
};

template <> struct fmt::formatter<KeyState> : formatter<string_view> {
  // parse is inherited from formatter<string_view>.

  auto format(KeyState c, format_context& ctx) const
      -> format_context::iterator;
};