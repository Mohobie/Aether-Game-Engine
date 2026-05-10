#include "platform/window.h"
#include <GLFW/glfw3.h>
#include <iostream>

namespace vge {

Window::Window() : window(nullptr), width(1280), height(720), shouldClose(false) {}

Window::~Window() {
    Shutdown();
}

bool Window::Initialize(int w, int h, const std::string& title) {
    width = w;
    height = h;
    
    if (!glfwInit()) {
        std::cerr << "[Window] GLFW initialization failed" << std::endl;
        return false;
    }
    
    // OpenGL 3.3 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Create window
    window = glfwCreateWindow(w, h, title.c_str(), nullptr, nullptr);
    if (!window) {
        std::cerr << "[Window] Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);
    
    // Set callbacks
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetCursorPosCallback(window, CursorCallback);
    glfwSetWindowCloseCallback(window, CloseCallback);
    
    std::cout << "[Window] Created " << w << "x" << h << " window" << std::endl;
    return true;
}

void Window::Shutdown() {
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
    std::cout << "[Window] Shutdown complete" << std::endl;
}

void Window::PollEvents() {
    glfwPollEvents();
    shouldClose = glfwWindowShouldClose(window);
}

void Window::SwapBuffers() {
    glfwSwapBuffers(window);
}

bool Window::ShouldClose() const {
    return shouldClose || glfwWindowShouldClose(window);
}

void* Window::GetHandle() const {
    return window;
}

bool Window::IsKeyPressed(int key) const {
    if (!window) return false;
    return glfwGetKey(window, key) == GLFW_PRESS;
}

void Window::GetMousePosition(double& x, double& y) const {
    if (window) {
        glfwGetCursorPos(window, &x, &y);
    }
}

bool Window::IsMouseButtonPressed(int button) const {
    if (!window) return false;
    return glfwGetMouseButton(window, button) == GLFW_PRESS;
}

void Window::SetCursorMode(bool locked) {
    if (window) {
        glfwSetInputMode(window, GLFW_CURSOR, 
            locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }
}

// Callbacks
void Window::KeyCallback(GLFWwindow* w, int key, int scancode, int action, int mods) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(w));
    if (win && win->inputCallback) {
        win->inputCallback(key, action);
    }
}

void Window::MouseButtonCallback(GLFWwindow* w, int button, int action, int mods) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(w));
    if (win && win->mouseCallback) {
        double x, y;
        glfwGetCursorPos(w, &x, &y);
        win->mouseCallback(button, action, x, y);
    }
}

void Window::CursorCallback(GLFWwindow* w, double x, double y) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(w));
    if (win && win->cursorCallback) {
        win->cursorCallback(x, y);
    }
}

void Window::CloseCallback(GLFWwindow* w) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(w));
    if (win) {
        win->shouldClose = true;
    }
}

} // namespace vge