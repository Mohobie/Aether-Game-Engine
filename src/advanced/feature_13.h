#pragma once
#include <vector>

namespace VoxelEngine {
    class AdvancedFeature13 {
    public:
        void Initialize();
        void Process(std::vector<float>& data);
        void Shutdown();
    };
}
