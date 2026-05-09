#pragma once

namespace VoxelEngine {
    class System9 {
    public:
        virtual void Initialize();
        virtual void Update(float deltaTime);
        virtual void Shutdown();
        virtual const char* GetName() const { return "System9"; }
    };
}
