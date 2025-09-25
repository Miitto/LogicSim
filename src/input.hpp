#pragma once

#include <gl/window.hpp>
#include <glm/glm.hpp>
#include <spdlog/fmt/bundled/format.h>
#include <unordered_map>

struct Mouse {
  glm::vec2 position{};
  glm::vec2 delta{};
  int buttons = 0;

  glm::vec2 lastPosition() const { return position - delta; }
  void onClick(int button) { buttons |= (1 << button); }
  void onRelease(int button) { buttons &= ~(1 << button); }
  bool isButtonDown(int button) const { return (buttons & (1 << button)) != 0; }
};

enum class KeyState { Pressed, PressedRepeat, Released };

class Input {
  gl::Window& m_window;
  Mouse m_mouse{};
  std::unordered_map<int, KeyState> keyState{};

  bool m_imguiWantsKeyboard = false;
  bool m_imguiWantsMouse = false;

public:
  Input(gl::Window& window) noexcept : m_window(window) {
    setupWindowCallbacks();
  }
  ~Input() = default;

  Input(const Input&) = delete;
  Input& operator=(const Input&) = delete;

  Input(Input&& other) noexcept : m_window(other.m_window) {
    m_window.setUserPtr(this);
  }

  void onKeyEvent(int key, int action);
  void onMouseMove(double x, double y);
  void onMouseButton(int button, int action);

  void imGuiWantsKeyboard(bool wants) { m_imguiWantsKeyboard = wants; }
  void imGuiWantsMouse(bool wants) { m_imguiWantsMouse = wants; }

  static void glfwKeyCallback(GLFWwindow* window, int key, int scancode,
                              int action, int mods);
  static void glfwCursorPosCallback(GLFWwindow* window, double xpos,
                                    double ypos);
  static void glfwMouseButtonCallback(GLFWwindow* window, int button,
                                      int action, int mods);

  void setupWindowCallbacks() {
    m_window.setUserPtr(this);
    m_window.setKeyCallback(Input::glfwKeyCallback);
    m_window.setCursorPosCallback(Input::glfwCursorPosCallback);
    m_window.setMouseButtonCallback(Input::glfwMouseButtonCallback);
  }

  KeyState getKeyState(int key) const {
    auto it = keyState.find(key);
    return it == keyState.end() ? KeyState::Released : it->second;
  }
  /// <summary>
  /// Returns if the key is currently held down (Pressed or PressedRepeat)
  /// </summary>
  /// <param name="key"></param>
  /// <returns></returns>
  bool isKeyDown(int key) const {
    auto it = keyState.find(key);
    return it != keyState.end() && (it->second == KeyState::Pressed ||
                                    it->second == KeyState::PressedRepeat);
  }
  /// <summary>
  /// Returns if the key is currently up (not pressed)
  /// </summary>
  /// <param name="key"></param>
  /// <returns></returns>
  bool isKeyUp(int key) const {
    auto it = keyState.find(key);
    return it == keyState.end() || it->second == KeyState::Released;
  }
  /// <summary>
  /// Returns if the key was pressed this frame (not held)
  /// </summary>
  /// <param name="key"></param>
  /// <returns></returns>
  bool isKeyPressed(int key) const {
    auto it = keyState.find(key);
    return it != keyState.end() && it->second == KeyState::Pressed;
  }
  /// <summary>
  /// Returns if the key was released this frame
  /// </summary>
  /// <param name="key"></param>
  /// <returns></returns>
  bool isKeyReleased(int key) const {
    auto it = keyState.find(key);
    return it != keyState.end() && it->second == KeyState::Released;
  }

  void frameEnd();

  const Mouse& mouse() const { return m_mouse; }
};

template <> struct fmt::formatter<KeyState> : formatter<string_view> {
  // parse is inherited from formatter<string_view>.

  auto format(KeyState c, format_context& ctx) const
      -> format_context::iterator;
};