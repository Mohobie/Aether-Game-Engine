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
    E, Q, F, Shift, Ctrl,
    Key0, Key1, Key2, Key3, Key4, Key5, Key6, Key7, Key8, Key9,
    Count
};

// Cross-platform input manager
class Input {
private:
    bool keys[static_cast<int>(KeyCode::Count)];
    bool prevKeys[static_cast<int>(KeyCode::Count)];
    float mouseDeltaX, mouseDeltaY;
    float scrollDelta;
    
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
    
    // Mouse input
    void SetMouseDelta(float dx, float dy);
    void GetMouseDelta(float& dx, float& dy) const;
    void SetScrollDelta(float delta);
    float GetScrollDelta() const;
    void ResetMouseDelta();
    
    // Legacy compatibility
    bool IsKeyPressed(int key) const;
    bool IsKeyJustPressed(int key) const;
    bool IsKeyReleased(int key) const;
};

} // namespace vge
