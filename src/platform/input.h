#pragma once

namespace VoxelEngine {
    class Input {
    public:
        bool IsKeyPressed(int key);
        bool IsMouseButtonPressed(int button);
        void GetMousePosition(double& x, double& y);
    };
}
