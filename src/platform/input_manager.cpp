#include "input_manager.h"
#include <iostream>
#include <cstring>

namespace vge {

Input::Input() {
    std::memset(keys, 0, sizeof(keys));
    std::memset(prevKeys, 0, sizeof(prevKeys));
    mouseDeltaX = 0;
    mouseDeltaY = 0;
    scrollDelta = 0;
    
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
            case 'f': case 'F': key = KeyCode::F; break;
            case '0': key = KeyCode::Key0; break;
            case '1': key = KeyCode::Key1; break;
            case '2': key = KeyCode::Key2; break;
            case '3': key = KeyCode::Key3; break;
            case '4': key = KeyCode::Key4; break;
            case '5': key = KeyCode::Key5; break;
            case '6': key = KeyCode::Key6; break;
            case '7': key = KeyCode::Key7; break;
            case '8': key = KeyCode::Key8; break;
            case '9': key = KeyCode::Key9; break;
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
    checkKey('F', KeyCode::F);
    checkKey(VK_SHIFT, KeyCode::Shift);
    checkKey(VK_CONTROL, KeyCode::Ctrl);
    checkKey('1', KeyCode::Key1);
    checkKey('2', KeyCode::Key2);
    checkKey('3', KeyCode::Key3);
    checkKey('4', KeyCode::Key4);
    checkKey('5', KeyCode::Key5);
    checkKey('6', KeyCode::Key6);
    checkKey('7', KeyCode::Key7);
    checkKey('8', KeyCode::Key8);
    checkKey('9', KeyCode::Key9);
    checkKey('0', KeyCode::Key0);
}
#endif

void Input::Update(void* windowHandle) {
    (void)windowHandle;
    // Copy current to previous
    std::memcpy(prevKeys, keys, sizeof(keys));
    std::memset(keys, 0, sizeof(keys));
    
    // Reset mouse delta each frame (must be set externally for now)
    mouseDeltaX = 0;
    mouseDeltaY = 0;
    scrollDelta = 0;
    
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

void Input::SetMouseDelta(float dx, float dy) {
    mouseDeltaX = dx;
    mouseDeltaY = dy;
}

void Input::GetMouseDelta(float& dx, float& dy) const {
    dx = mouseDeltaX;
    dy = mouseDeltaY;
}

void Input::SetScrollDelta(float delta) {
    scrollDelta = delta;
}

float Input::GetScrollDelta() const {
    return scrollDelta;
}

void Input::ResetMouseDelta() {
    mouseDeltaX = 0;
    mouseDeltaY = 0;
    scrollDelta = 0;
}

} // namespace vge
