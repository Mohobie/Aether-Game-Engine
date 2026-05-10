#pragma once
#include <string>
#include <functional>
#include <map>

#ifdef PLATFORM_WINDOWS
    #include <windows.h>
#else
    #include <termios.h>
    #include <unistd.h>
    #include <fcntl.h>
#endif

namespace vge {

// Cross-platform key codes
enum class KeyCode {
    W = 0, A, S, D, Space, Escape, Enter, 
    Up, Down, Left, Right,
    E, Q, Shift, Ctrl,
    Count
};

// Cross-platform input manager
class Input {
private:
    bool keys[static_cast<int>(KeyCode::Count)];
    bool prevKeys[static_cast<int>(KeyCode::Count)];
    
#ifdef PLATFORM_LINUX
    struct termios originalTermios;
    bool terminalMode;
    int stdin_fd;
#endif
    
    void EnableRawMode();
    void DisableRawMode();
    void ProcessKeyboardInput();
    
public:
    Input();
    ~Input();
    
    void Update(void* windowHandle = nullptr);
    
    bool IsKeyPressed(KeyCode key) const;
    bool IsKeyJustPressed(KeyCode key) const;
    bool IsKeyReleased(KeyCode key) const;
    
    // Legacy compatibility
    bool IsKeyPressed(int key) const;
    bool IsKeyJustPressed(int key) const;
    bool IsKeyReleased(int key) const;
};

} // namespace vge
