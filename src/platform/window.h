#pragma once

// Iteration 4
namespace VoxelEngine {
    class Window {
    public:
        void Create(int width, int height, const char* title);
        void PollEvents();
        void SwapBuffers();
        bool ShouldClose();
        void Destroy();
    };
}
