#pragma once

namespace VoxelEngine {
    class ImGuiWrapper {
    public:
        void Initialize();
        void BeginFrame();
        void EndFrame();
        void Render();
        void Shutdown();
    };
}
