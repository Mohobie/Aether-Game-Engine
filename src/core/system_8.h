#pragma once

namespace VoxelEngine {
    class System8 {
    public:
        virtual void Initialize();
        virtual void Update(float deltaTime);
        virtual void Shutdown();
        virtual const char* GetName() const { return "System8"; }
    };
}
