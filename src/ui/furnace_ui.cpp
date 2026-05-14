#include "ui/furnace_ui.h"
#include "core/inventory.h"
#include "core/survival_system.h"
#include "core/logger.h"

namespace vge {

FurnaceUI::FurnaceUI(FurnaceManager* manager, Inventory* inventory, SurvivalSystem* survival)
    : furnaceManager(manager), playerInventory(inventory), survivalSystem(survival),
      currentFurnaceX(0), currentFurnaceY(0), currentFurnaceZ(0), isOpen(false) {}

void FurnaceUI::OpenFurnace(int x, int y, int z) {
    if (!furnaceManager || !furnaceManager->HasFurnace(x, y, z)) return;
    currentFurnaceX = x;
    currentFurnaceY = y;
    currentFurnaceZ = z;
    isOpen = true;
    Logger::Info("[FurnaceUI] Opened furnace at (" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")");
}

void FurnaceUI::CloseFurnace() {
    isOpen = false;
    Logger::Info("[FurnaceUI] Closed furnace");
}

void FurnaceUI::GetCurrentFurnace(int& x, int& y, int& z) const {
    x = currentFurnaceX;
    y = currentFurnaceY;
    z = currentFurnaceZ;
}

bool FurnaceUI::PutInput(const std::string& itemID, int count) {
    if (!isOpen || !furnaceManager) return false;
    return furnaceManager->AddInput(currentFurnaceX, currentFurnaceY, currentFurnaceZ, itemID, count);
}

bool FurnaceUI::PutFuel(const std::string& itemID, int count) {
    if (!isOpen || !furnaceManager) return false;
    return furnaceManager->AddFuel(currentFurnaceX, currentFurnaceY, currentFurnaceZ, itemID, count);
}

bool FurnaceUI::TakeOutput(int count) {
    if (!isOpen || !furnaceManager) return false;
    // Collect experience when taking output
    float xp = furnaceManager->CollectExperience(currentFurnaceX, currentFurnaceY, currentFurnaceZ);
    if (xp > 0.0f && survivalSystem) {
        survivalSystem->AddExperience((int)xp);
    }
    return furnaceManager->RemoveOutput(currentFurnaceX, currentFurnaceY, currentFurnaceZ, count);
}

bool FurnaceUI::TakeInput(int count) {
    if (!isOpen || !furnaceManager) return false;
    return furnaceManager->TakeInput(currentFurnaceX, currentFurnaceY, currentFurnaceZ, count);
}

bool FurnaceUI::TakeFuel(int count) {
    if (!isOpen || !furnaceManager) return false;
    return furnaceManager->TakeFuel(currentFurnaceX, currentFurnaceY, currentFurnaceZ, count);
}

float FurnaceUI::CollectExperience() {
    if (!isOpen || !furnaceManager) return 0.0f;
    float xp = furnaceManager->CollectExperience(currentFurnaceX, currentFurnaceY, currentFurnaceZ);
    if (xp > 0.0f && survivalSystem) {
        survivalSystem->AddExperience((int)xp);
    }
    return xp;
}

bool FurnaceUI::GetFurnaceState(std::string& inputItem, int& inputCount,
                                std::string& fuelItem, int& fuelCount,
                                std::string& outputItem, int& outputCount,
                                float& smeltProgress, float& fuelProgress,
                                bool& isLit, float& storedXP) {
    if (!isOpen || !furnaceManager) return false;

    FurnaceBlockState* furnace = furnaceManager->GetFurnace(currentFurnaceX, currentFurnaceY, currentFurnaceZ);
    if (!furnace) return false;

    inputItem = furnace->inputItem;
    inputCount = furnace->inputCount;
    fuelItem = furnace->fuelItem;
    fuelCount = furnace->fuelCount;
    outputItem = furnace->outputItem;
    outputCount = furnace->outputCount;
    smeltProgress = furnace->smeltProgress;
    fuelProgress = furnaceManager->GetFuelProgress(currentFurnaceX, currentFurnaceY, currentFurnaceZ);
    isLit = furnace->isLit;
    storedXP = furnace->storedExperience;
    return true;
}

void FurnaceUI::Update(float deltaTime) {
    (void)deltaTime;
    // UI state is synced from furnace manager each frame
}

} // namespace vge
