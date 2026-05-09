#pragma once

namespace VoxelEngine {
    class Renderer {
    public:
        void Initialize();
        void Render();
        void Shutdown();
        void SetViewport(int x, int y, int width, int height);
    };
}
