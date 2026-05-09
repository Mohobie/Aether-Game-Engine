#include "input.h"

namespace vge {
void Input::Update(GLFWwindow* window) {
    for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; ++key) {
        keys[key] = glfwGetKey(window, key) == GLFW_PRESS;
    }
    for (int btn = GLFW_MOUSE_BUTTON_1; btn <= GLFW_MOUSE_BUTTON_LAST; ++btn) {
        mouseButtons[btn] = glfwGetMouseButton(window, btn) == GLFW_PRESS;
    }
    glfwGetCursorPos(window, &mouseX, &mouseY);
}

bool Input::IsKeyPressed(int key) const {
    auto it = keys.find(key);
    return it != keys.end() && it->second;
}

bool Input::IsMousePressed(int button) const {
    auto it = mouseButtons.find(button);
    return it != mouseButtons.end() && it->second;
}

void Input::GetMousePos(double& x, double& y) const {
    x = mouseX; y = mouseY;
}
}