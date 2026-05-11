#include "platform/input.h"
#include "platform/window.h"
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <cstring>

// Terminal-based input implementation (no GLFW needed)
// Supports both press and hold modes

namespace vge {

Input::Input() : window(nullptr), terminalMode(false), stdin_fd(STDIN_FILENO) {
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
    prevMouseX = prevMouseY = 0;
    mouseDeltaX = mouseDeltaY = 0;
    scrollDelta = 0;
}

Input::~Input() {
    if (terminalMode) {
        DisableTerminalMode();
    }
}

void Input::EnableTerminalMode() {
    if (terminalMode) return;
    
    struct termios tty;
    tcgetattr(stdin_fd, &tty);
    tty.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(stdin_fd, TCSANOW, &tty);
    
    int flags = fcntl(stdin_fd, F_GETFL, 0);
    fcntl(stdin_fd, F_SETFL, flags | O_NONBLOCK);
    
    terminalMode = true;
    std::cout << "[Input] Terminal mode enabled" << std::endl;
}

void Input::DisableTerminalMode() {
    if (!terminalMode) return;
    
    struct termios tty;
    tcgetattr(stdin_fd, &tty);
    tty.c_lflag |= (ICANON | ECHO);
    tcsetattr(stdin_fd, TCSANOW, &tty);
    
    terminalMode = false;
    std::cout << "[Input] Terminal mode disabled" << std::endl;
}

void Input::Update(void* windowHandle) {
    // Copy current to previous
    for (int i = 0; i < 512; ++i) {
        prevKeys[i] = keys[i];
    }
    for (int i = 0; i < 8; ++i) {
        prevMouseButtons[i] = mouseButtons[i];
    }
    prevMouseX = mouseX;
    prevMouseY = mouseY;
    mouseDeltaX = 0;
    mouseDeltaY = 0;
    scrollDelta = 0;
    
    // Enable terminal mode on first update
    if (!terminalMode) {
        EnableTerminalMode();
    }
    
    // Read keyboard input from terminal
    char c;
    bool keyPressedThisFrame[512] = {false};
    
    while (read(stdin_fd, &c, 1) > 0) {
        int keyCode = -1;
        
        switch (c) {
            case 'w': case 'W': keyCode = GLFW_KEY_W; break;
            case 'a': case 'A': keyCode = GLFW_KEY_A; break;
            case 's': case 'S': keyCode = GLFW_KEY_S; break;
            case 'd': case 'D': keyCode = GLFW_KEY_D; break;
            case ' ': keyCode = GLFW_KEY_SPACE; break;
            case 'e': case 'E': keyCode = GLFW_KEY_E; break;
            case 'q': case 'Q': keyCode = GLFW_KEY_Q; break;
            case 27: // Escape or arrow key sequence
                char seq[2];
                if (read(stdin_fd, &seq[0], 1) > 0) {
                    if (read(stdin_fd, &seq[1], 1) > 0) {
                        if (seq[0] == '[') {
                            switch (seq[1]) {
                                case 'A': keyCode = GLFW_KEY_UP; break;
                                case 'B': keyCode = GLFW_KEY_DOWN; break;
                                case 'C': keyCode = GLFW_KEY_RIGHT; break;
                                case 'D': keyCode = GLFW_KEY_LEFT; break;
                            }
                        }
                    } else {
                        keyCode = GLFW_KEY_ESCAPE;
                    }
                } else {
                    keyCode = GLFW_KEY_ESCAPE;
                }
                break;
        }
        
        if (keyCode >= 0 && keyCode < 512) {
            keys[keyCode] = true;
            keyPressedThisFrame[keyCode] = true;
        }
    }
    
    // For keys not pressed this frame, keep them pressed if they were held
    // But allow release detection by checking if they were in prevKeys
    // This is a compromise - we can't detect true key release in terminal
    // So we auto-release after a short delay (simulated by frame count)
    static int keyHoldFrames[512] = {0};
    for (int i = 0; i < 512; ++i) {
        if (keys[i]) {
            if (keyPressedThisFrame[i]) {
                keyHoldFrames[i] = 0; // Reset on new press
            } else {
                keyHoldFrames[i]++;
                // Auto-release after 5 frames (about 83ms at 60fps)
                if (keyHoldFrames[i] > 5) {
                    keys[i] = false;
                    keyHoldFrames[i] = 0;
                }
            }
        }
    }
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

bool Input::IsKeyReleased(int key) const {
    if (key >= 0 && key < 512) {
        return !keys[key] && prevKeys[key];
    }
    return false;
}

bool Input::IsMouseButtonPressed(int button) const {
    if (button >= 0 && button < 8) {
        return mouseButtons[button];
    }
    return false;
}

bool Input::IsMouseButtonJustPressed(int button) const {
    if (button >= 0 && button < 8) {
        return mouseButtons[button] && !prevMouseButtons[button];
    }
    return false;
}

bool Input::IsMouseButtonReleased(int button) const {
    if (button >= 0 && button < 8) {
        return !mouseButtons[button] && prevMouseButtons[button];
    }
    return false;
}

void Input::GetMousePosition(double& x, double& y) const {
    x = mouseX;
    y = mouseY;
}

void Input::GetMouseDelta(double& dx, double& dy) const {
    dx = mouseDeltaX;
    dy = mouseDeltaY;
}

double Input::GetScrollDelta() const {
    return scrollDelta;
}

} // namespace vge