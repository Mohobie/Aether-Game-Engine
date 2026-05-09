#include "platform/input.h"
#include "platform/window.h"
#include <iostream>

// Stub implementation when GLFW is not available
namespace vge {

Input::Input() : window(nullptr) {
    // Initialize key states
    for (int i = 0; i < 512; ++i) {
        keys[i] = false;
        prevKeys[i] = false;
    }
    for (int i = 0; i < 8; ++i) {
        mouseButtons[i] = false;
        prevMouseButtons[i] = false;
    }
    mouseX = mouseY = 0;
}

void Input::Update(void* windowHandle) {
    // Copy current to previous
    for (int i = 0; i < 512; ++i) {
        prevKeys[i] = keys[i];
    }
    for (int i = 0; i < 8; ++i) {
        prevMouseButtons[i] = mouseButtons[i];
    }
    
    // In a real implementation, would poll GLFW input here
    // For now, just stub
}

bool Input::IsKeyPressed(int key) const {
    if (key >= 0 && key < 512) {
        return keys[key];
    }
    return false;
}

bool Input::IsKeyJustPressed(int key) const {
    if (key >= 0 && key < 512) {
        return keys[key] && !prevKeys[key];
    }
    return false;
}

bool Input::IsMouseButtonPressed(int button) const {
    if (button >= 0 && button < 8) {
        return mouseButtons[button];
    }
    return false;
}

void Input::GetMousePosition(double& x, double& y) const {
    x = mouseX;
    y = mouseY;
}

} // namespace vge