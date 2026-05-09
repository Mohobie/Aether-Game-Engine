#include "input_manager.h"
namespace aether {
InputManager::InputManager() {}
void InputManager::update() {
    previousKeyStates = keyStates;
    lastMouseX = mouseX;
    lastMouseY = mouseY;
}
bool InputManager::isKeyDown(Key key) const {
    auto it = keyStates.find(static_cast<int>(key));
    return it != keyStates.end() && it->second;
}
bool InputManager::isKeyPressed(Key key) const {
    int k = static_cast<int>(key);
    auto current = keyStates.find(k);
    auto previous = previousKeyStates.find(k);
    bool wasDown = (previous != previousKeyStates.end() && previous->second);
    bool isDown = (current != keyStates.end() && current->second);
    return isDown && !wasDown;
}
bool InputManager::isKeyReleased(Key key) const {
    int k = static_cast<int>(key);
    auto current = keyStates.find(k);
    auto previous = previousKeyStates.find(k);
    bool wasDown = (previous != previousKeyStates.end() && previous->second);
    bool isDown = (current != keyStates.end() && current->second);
    return !isDown && wasDown;
}
bool InputManager::isMouseButtonDown(MouseButton button) const {
    auto it = mouseButtonStates.find(static_cast<int>(button));
    return it != mouseButtonStates.end() && it->second;
}
void InputManager::getMousePosition(float& x, float& y) const { x = mouseX; y = mouseY; }
void InputManager::getMouseDelta(float& dx, float& dy) const { dx = mouseX - lastMouseX; dy = mouseY - lastMouseY; }
void InputManager::bindKey(Key key, std::function<void()> callback) {}
} // namespace aether
