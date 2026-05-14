#pragma once
#include "game/furnace_system.h"
#include <string>

namespace vge {

// Forward declarations
class Inventory;
class SurvivalSystem;

// ============================================
// Furnace UI - Simple text-based interface
// ============================================
class FurnaceUI {
private:
    FurnaceManager* furnaceManager;
    Inventory* playerInventory;
    SurvivalSystem* survivalSystem;
    
    int currentFurnaceX;
    int currentFurnaceY;
    int currentFurnaceZ;
    bool isOpen;

public:
    FurnaceUI(FurnaceManager* manager, Inventory* inventory, SurvivalSystem* survival);

    // Open/close furnace at position
    void OpenFurnace(int x, int y, int z);
    void CloseFurnace();
    bool IsOpen() const { return isOpen; }

    // Get current furnace position
    void GetCurrentFurnace(int& x, int& y, int& z) const;

    // Item operations (called from game logic)
    bool PutInput(const std::string& itemID, int count);
    bool PutFuel(const std::string& itemID, int count);
    bool TakeOutput(int count);
    bool TakeInput(int count);
    bool TakeFuel(int count);
    float CollectExperience();

    // Get furnace state for rendering
    bool GetFurnaceState(std::string& inputItem, int& inputCount,
                         std::string& fuelItem, int& fuelCount,
                         std::string& outputItem, int& outputCount,
                         float& smeltProgress, float& fuelProgress,
                         bool& isLit, float& storedXP);

    // Update (sync with furnace manager)
    void Update(float deltaTime);
};

} // namespace vge
