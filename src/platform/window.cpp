#include "window.h"
#include <GLFW/glfw3.h>
#include <iostream>

namespace vge {

Window::Window() : window(nullptr), width(1280), height(720), title("Voxel Engine") {}

Window::~Window() {
    Shutdown();
}

bool Window::Initialize(int w, int h, const std::string& t) {
    width = w;
    height = h;
    title = t;
    
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);
    glfwSwapInterval(1);
    
    return true;
}

void Window::Shutdown() {
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
}

bool Window::ShouldClose() const {
    return window ? glfwWindowShouldClose(window) : true;
}

void Window::SwapBuffers() {
    if (window) glfwSwapBuffers(window);
}

void Window::PollEvents() {
    glfwPollEvents();
}

void Window::GetSize(int& w, int& h) const {
    w = width;
    h = height;
}

bool Window::IsKeyPressed(int key) const {
    return window ? glfwGetKey(window, key) == GLFW_PRESS : false;
}

} // namespace vge
