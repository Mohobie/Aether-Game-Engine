#pragma once
#include "math/vec3.h"
#include "voxel/block_types.h"
#include <vector>
#include <string>

namespace vge {

// ============================================
// Editor Test Suite
// ============================================
class EditorTest {
public:
    static bool RunAllTests();
    
    // Fly Camera Tests
    static bool TestFlyCameraMovement();
    static bool TestFlyCameraRotation();
    static bool TestFlyCameraLookAt();
    
    // Block Picker Tests
    static bool TestBlockPickerRaycast();
    static bool TestBlockPickerReachDistance();
    
    // Entity Spawner Tests
    static bool TestEntitySpawn();
    static bool TestEntityRemove();
    static bool TestEntityGridSpawn();
    
    // Selection Tests
    static bool TestBlockSelection();
    static bool TestEntitySelection();
    static bool TestSelectionClear();
    
    // Undo/Redo Tests
    static bool TestUndoRedo();
    static bool TestUndoStackLimit();
    
    // Integration Tests
    static bool TestEditorLifecycle();
    static bool TestEditorInputHandling();
};

} // namespace vge
