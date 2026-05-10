#pragma once
#include <string>
#include <functional>

struct GLFWwindow;

namespace vge {

class Window {
private:
    GLFWwindow* window;
    int width;
    int height;
    bool shouldClose;
    
    std::function<void(int, int)> inputCallback;
    std::function<void(int, int, double, double)> mouseCallback;
    std::function<void(double, double)> cursorCallback;
    
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void CursorCallback(GLFWwindow* window, double x, double y);
    static void CloseCallback(GLFWwindow* window);
    
public:
    Window();
    ~Window();
    
    bool Initialize(int width, int height, const std::string& title);
    void Shutdown();
    
    void PollEvents();
    void SwapBuffers();
    bool ShouldClose() const;
    
    void* GetHandle() const;
    
    bool IsKeyPressed(int key) const;
    void GetMousePosition(double& x, double& y) const;
    bool IsMouseButtonPressed(int button) const;
    void SetCursorMode(bool locked);
    
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    
    void SetInputCallback(std::function<void(int, int)> cb) { inputCallback = cb; }
    void SetMouseCallback(std::function<void(int, int, double, double)> cb) { mouseCallback = cb; }
    void SetCursorCallback(std::function<void(double, double)> cb) { cursorCallback = cb; }
};

} // namespace vge