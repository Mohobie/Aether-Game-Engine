#pragma once
#include <vector>

namespace VoxelEngine {
    class AdvancedFeature11 {
    public:
        void Initialize();
        void Process(std::vector<float>& data);
        void Shutdown();
    };
}
