#include "input_manager.h"
#include <iostream>
#include <cstring>

namespace vge {

Input::Input() {
    std::memset(keys, 0, sizeof(keys));
    std::memset(prevKeys, 0, sizeof(prevKeys));
    
#ifdef PLATFORM_LINUX
    terminalMode = false;
    stdin_fd = STDIN_FILENO;
#endif
}

Input::~Input() {
#ifdef PLATFORM_LINUX
    if (terminalMode) {
        DisableRawMode();
    }
#endif
}

#ifdef PLATFORM_LINUX
void Input::EnableRawMode() {
    if (terminalMode) return;
    
    tcgetattr(stdin_fd, &originalTermios);
    struct termios raw = originalTermios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(stdin_fd, TCSAFLUSH, &raw);
    
    int flags = fcntl(stdin_fd, F_GETFL, 0);
    fcntl(stdin_fd, F_SETFL, flags | O_NONBLOCK);
    
    terminalMode = true;
}

void Input::DisableRawMode() {
    if (!terminalMode) return;
    tcsetattr(stdin_fd, TCSAFLUSH, &originalTermios);
    terminalMode = false;
}

void Input::ProcessKeyboardInput() {
    if (!terminalMode) {
        EnableRawMode();
    }
    
    char c;
    while (read(stdin_fd, &c, 1) > 0) {
        KeyCode key = KeyCode::Count;
        
        switch (c) {
            case 'w': case 'W': key = KeyCode::W; break;
            case 'a': case 'A': key = KeyCode::A; break;
            case 's': case 'S': key = KeyCode::S; break;
            case 'd': case 'D': key = KeyCode::D; break;
            case ' ': key = KeyCode::Space; break;
            case 'e': case 'E': key = KeyCode::E; break;
            case 'q': case 'Q': key = KeyCode::Q; break;
            case 27: // Escape or arrow keys
                char seq[2];
                if (read(stdin_fd, &seq[0], 1) > 0) {
                    if (read(stdin_fd, &seq[1], 1) > 0) {
                        if (seq[0] == '[') {
                            switch (seq[1]) {
                                case 'A': key = KeyCode::Up; break;
                                case 'B': key = KeyCode::Down; break;
                                case 'C': key = KeyCode::Right; break;
                                case 'D': key = KeyCode::Left; break;
                            }
                        }
                    } else {
                        key = KeyCode::Escape;
                    }
                } else {
                    key = KeyCode::Escape;
                }
                break;
        }
        
        if (key != KeyCode::Count) {
            keys[static_cast<int>(key)] = true;
        }
    }
}
#endif

#ifdef PLATFORM_WINDOWS
void Input::ProcessKeyboardInput() {
    // Windows: Use GetAsyncKeyState for real-time key detection
    auto checkKey = [this](int vk, KeyCode code) {
        if (GetAsyncKeyState(vk) & 0x8000) {
            keys[static_cast<int>(code)] = true;
        }
    };
    
    checkKey('W', KeyCode::W);
    checkKey('A', KeyCode::A);
    checkKey('S', KeyCode::S);
    checkKey('D', KeyCode::D);
    checkKey(VK_SPACE, KeyCode::Space);
    checkKey(VK_ESCAPE, KeyCode::Escape);
    checkKey(VK_RETURN, KeyCode::Enter);
    checkKey(VK_UP, KeyCode::Up);
    checkKey(VK_DOWN, KeyCode::Down);
    checkKey(VK_LEFT, KeyCode::Left);
    checkKey(VK_RIGHT, KeyCode::Right);
    checkKey('E', KeyCode::E);
    checkKey('Q', KeyCode::Q);
    checkKey(VK_SHIFT, KeyCode::Shift);
    checkKey(VK_CONTROL, KeyCode::Ctrl);
}
#endif

void Input::Update() {
    // Copy current to previous
    std::memcpy(prevKeys, keys, sizeof(keys));
    std::memset(keys, 0, sizeof(keys));
    
    ProcessKeyboardInput();
}

bool Input::IsKeyPressed(KeyCode key) const {
    int idx = static_cast<int>(key);
    if (idx >= 0 && idx < static_cast<int>(KeyCode::Count)) {
        return keys[idx];
    }
    return false;
}

bool Input::IsKeyJustPressed(KeyCode key) const {
    int idx = static_cast<int>(key);
    if (idx >= 0 && idx < static_cast<int>(KeyCode::Count)) {
        return keys[idx] && !prevKeys[idx];
    }
    return false;
}

bool Input::IsKeyReleased(KeyCode key) const {
    int idx = static_cast<int>(key);
    if (idx >= 0 && idx < static_cast<int>(KeyCode::Count)) {
        return !keys[idx] && prevKeys[idx];
    }
    return false;
}

// Legacy compatibility
bool Input::IsKeyPressed(int key) const {
    if (key >= 0 && key < static_cast<int>(KeyCode::Count)) {
        return keys[key];
    }
    return false;
}

bool Input::IsKeyJustPressed(int key) const {
    if (key >= 0 && key < static_cast<int>(KeyCode::Count)) {
        return keys[key] && !prevKeys[key];
    }
    return false;
}

bool Input::IsKeyReleased(int key) const {
    if (key >= 0 && key < static_cast<int>(KeyCode::Count)) {
        return !keys[key] && prevKeys[key];
    }
    return false;
}

} // namespace vge
