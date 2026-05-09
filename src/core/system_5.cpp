#include "system_5.h"
#include <iostream>

namespace VoxelEngine {
    void System5::Initialize() {
        std::cout << "System 5 initialized" << std::endl;
    }
    
    void System5::Update(float deltaTime) {
        // System 5 update
    }
    
    void System5::Shutdown() {
        std::cout << "System 5 shutdown" << std::endl;
    }
}
