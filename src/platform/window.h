#pragma once
#include <string>

namespace vge {
    class Window {
    private:
        void* window;
        int width;
        int height;
        bool shouldClose;
        
    public:
        Window();
        ~Window();
        
        bool Initialize(int width, int height, const std::string& title);
        void Shutdown();
        
        void PollEvents();
        void SwapBuffers();
        bool ShouldClose() const;
        void* GetHandle() const;
        
        int GetWidth() const { return width; }
        int GetHeight() const { return height; }
    };
}