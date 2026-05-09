#include "platform/input.h"
#include "platform/window.h"
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>

// Terminal-based input implementation (no GLFW needed)
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
    
    // Set terminal to non-blocking mode
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
    
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}

void Input::Update(void* windowHandle) {
    // Copy current to previous
    for (int i = 0; i < 512; ++i) {
        prevKeys[i] = keys[i];
    }
    for (int i = 0; i < 8; ++i) {
        prevMouseButtons[i] = mouseButtons[i];
    }
    
    // Read keyboard input from terminal
    char c;
    while (read(STDIN_FILENO, &c, 1) > 0) {
        switch (c) {
            case 'w': case 'W': keys[GLFW_KEY_W] = true; break;
            case 'a': case 'A': keys[GLFW_KEY_A] = true; break;
            case 's': case 'S': keys[GLFW_KEY_S] = true; break;
            case 'd': case 'D': keys[GLFW_KEY_D] = true; break;
            case ' ': keys[GLFW_KEY_SPACE] = true; break;
            case 'e': case 'E': keys[GLFW_KEY_E] = true; break;
            case 'q': case 'Q': keys[GLFW_KEY_Q] = true; break;
            case 27: // Escape
                // Check for arrow keys (escape sequences)
                char seq[2];
                if (read(STDIN_FILENO, &seq[0], 1) > 0) {
                    if (read(STDIN_FILENO, &seq[1], 1) > 0) {
                        if (seq[0] == '[') {
                            switch (seq[1]) {
                                case 'A': keys[GLFW_KEY_UP] = true; break;    // Up
                                case 'B': keys[GLFW_KEY_DOWN] = true; break;  // Down
                                case 'C': keys[GLFW_KEY_RIGHT] = true; break; // Right
                                case 'D': keys[GLFW_KEY_LEFT] = true; break;  // Left
                            }
                        }
                    } else {
                        keys[GLFW_KEY_ESCAPE] = true;
                    }
                } else {
                    keys[GLFW_KEY_ESCAPE] = true;
                }
                break;
        }
    }
    
    // Auto-release keys (since terminal doesn't send key-up events)
    // We'll release them on the next frame unless pressed again
    // This creates a "press once per frame" behavior
    for (int i = 0; i < 512; ++i) {
        if (keys[i] && !prevKeys[i]) {
            // Key was just pressed, schedule release
            keys[i] = false; // Auto-release for next frame
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