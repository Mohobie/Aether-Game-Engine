#pragma once
#include <string>
#include <unordered_map>
#include <functional>
namespace vge {
enum class Key {
    Unknown = -1, Space = 32, A = 65, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    Escape = 256, Enter, Tab, Backspace, Delete,
    Right = 262, Left, Down, Up,
    Shift = 340, Control, Alt
};
enum class MouseButton { Left = 0, Right = 1, Middle = 2 };
class InputManager {
public:
    InputManager();
    void update();
    bool isKeyDown(Key key) const;
    bool isKeyPressed(Key key) const;
    bool isKeyReleased(Key key) const;
    bool isMouseButtonDown(MouseButton button) const;
    void getMousePosition(float& x, float& y) const;
    void getMouseDelta(float& dx, float& dy) const;
    void bindKey(Key key, std::function<void()> callback);
private:
    std::unordered_map<int, bool> keyStates;
    std::unordered_map<int, bool> previousKeyStates;
    std::unordered_map<int, bool> mouseButtonStates;
    float mouseX = 0, mouseY = 0;
    float lastMouseX = 0, lastMouseY = 0;
};
} // namespace vge
