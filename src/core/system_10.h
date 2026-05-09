#pragma once

namespace VoxelEngine {
    class System10 {
    public:
        virtual void Initialize();
        virtual void Update(float deltaTime);
        virtual void Shutdown();
        virtual const char* GetName() const { return "System10"; }
    };
}
