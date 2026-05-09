#include "platform/window.h"
#include <iostream>

// Stub implementation when GLFW is not available
namespace vge {

Window::Window() : window(nullptr), width(1280), height(720), shouldClose(false) {}

Window::~Window() {
    Shutdown();
}

bool Window::Initialize(int w, int h, const std::string& title) {
    width = w;
    height = h;
    std::cout << "[Window] Stub - would create " << w << "x" << h << " window" << std::endl;
    return true;
}

void Window::Shutdown() {
    std::cout << "[Window] Stub - shutting down" << std::endl;
}

void Window::PollEvents() {
    // Stub - no events without GLFW
}

void Window::SwapBuffers() {
    // Stub - no buffer swapping without GLFW
}

bool Window::ShouldClose() const {
    return shouldClose;
}

void* Window::GetHandle() const {
    return window;
}

} // namespace vge