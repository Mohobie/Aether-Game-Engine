#include "ui_inventory.h"
#include "core/item_system.h"
#include "core/logger.h"
#include "voxel/block_registry.h"
#include <algorithm>

namespace vge {

// ==================== UIInventorySlot ====================

UIInventorySlot::UIInventorySlot(int index, const std::string& invID)
    : slotIndex(index), inventoryID(invID), isHovered(false), isPressed(false),
      isDragTarget(false), backgroundColor(0x444444FF), hoverColor(0x666666FF),
      pressedColor(0x888888FF), borderColor(0x222222FF), dragTargetColor(0x44AA44FF) {
    setSize(Vec2(50, 50));
}

void UIInventorySlot::setColors(uint32_t bg, uint32_t hover, uint32_t pressed, uint32_t border) {
    backgroundColor = bg;
    hoverColor = hover;
    pressedColor = pressed;
    borderColor = border;
}

void UIInventorySlot::render() {
    if (!isVisible()) return;
    
    // Determine color based on state
    uint32_t color = backgroundColor;
    if (isDragTarget) {
        color = dragTargetColor;
    } else if (isPressed) {
        color = pressedColor;
    } else if (isHovered) {
        color = hoverColor;
    }
    
    // TODO: Render slot background, item icon, and count text
    // This is a placeholder - actual rendering depends on the rendering system
    (void)color; // Suppress unused warning for now
}

bool UIInventorySlot::onClick(const Vec2& pos) {
    if (!isVisible()) return false;
    
    Vec2 slotPos = getPosition();
    Vec2 slotSize = getSize();
    
    if (pos.x >= slotPos.x && pos.x <= slotPos.x + slotSize.x &&
        pos.y >= slotPos.y && pos.y <= slotPos.y + slotSize.y) {
        
        if (onSlotClick) {
            onSlotClick(slotIndex);
        }
        return true;
    }
    
    return false;
}

// ==================== UIInventoryGrid ====================

UIInventoryGrid::UIInventoryGrid(const std::string& invID, int width, int height)
    : inventoryID(invID), gridWidth(width), gridHeight(height),
      slotSize(50.0f), slotSpacing(5.0f),
      backgroundColor(0x333333FF), slotBackgroundColor(0x444444FF),
      slotHoverColor(0x666666FF), slotPressedColor(0x888888FF), slotBorderColor(0x222222FF) {
    
    // Calculate grid size
    float totalWidth = width * slotSize + (width - 1) * slotSpacing;
    float totalHeight = height * slotSize + (height - 1) * slotSpacing;
    setSize(Vec2(totalWidth, totalHeight));
    
    // Create slot elements
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = y * width + x;
            auto slot = std::make_shared<UIInventorySlot>(index, invID);
            
            float posX = x * (slotSize + slotSpacing);
            float posY = y * (slotSize + slotSpacing);
            slot->setPosition(Vec2(posX, posY));
            slot->setSize(Vec2(slotSize, slotSize));
            slot->setColors(slotBackgroundColor, slotHoverColor, slotPressedColor, slotBorderColor);
            
            // Set up callbacks
            slot->onSlotClick = [this](int idx) {
                if (onSlotClicked) onSlotClicked(idx);
            };
            
            slot->onSlotHoverStart = [this](int idx) {
                if (onSlotHovered) onSlotHovered(idx);
            };
            
            slotElements.push_back(slot);
            addChild(slot);
        }
    }
}

void UIInventoryGrid::setSlotColors(uint32_t bg, uint32_t hover, uint32_t pressed, uint32_t border) {
    slotBackgroundColor = bg;
    slotHoverColor = hover;
    slotPressedColor = pressed;
    slotBorderColor = border;
    
    for (auto& slot : slotElements) {
        slot->setColors(bg, hover, pressed, border);
    }
}

void UIInventoryGrid::refreshSlots() {
    // Update slot visuals based on inventory data
    // TODO: Integrate with actual inventory data
}

std::shared_ptr<UIInventorySlot> UIInventoryGrid::getSlotElement(int index) {
    if (index >= 0 && index < (int)slotElements.size()) {
        return slotElements[index];
    }
    return nullptr;
}

std::shared_ptr<UIInventorySlot> UIInventoryGrid::getSlotElement(int x, int y) {
    int index = y * gridWidth + x;
    return getSlotElement(index);
}

int UIInventoryGrid::getSlotAtPosition(const Vec2& pos) const {
    Vec2 gridPos = getPosition();
    Vec2 localPos = Vec2(pos.x - gridPos.x, pos.y - gridPos.y);
    
    for (int y = 0; y < gridHeight; ++y) {
        for (int x = 0; x < gridWidth; ++x) {
            float slotX = x * (slotSize + slotSpacing);
            float slotY = y * (slotSize + slotSpacing);
            
            if (localPos.x >= slotX && localPos.x <= slotX + slotSize &&
                localPos.y >= slotY && localPos.y <= slotY + slotSize) {
                return y * gridWidth + x;
            }
        }
    }
    
    return -1;
}

Vec2 UIInventoryGrid::getSlotPosition(int index) const {
    int x = index % gridWidth;
    int y = index / gridWidth;
    
    Vec2 gridPos = getPosition();
    float slotX = gridPos.x + x * (slotSize + slotSpacing);
    float slotY = gridPos.y + y * (slotSize + slotSpacing);
    
    return Vec2(slotX, slotY);
}

void UIInventoryGrid::render() {
    if (!isVisible()) return;
    
    // Render grid background
    // TODO: Actual rendering implementation
    
    // Render slots
    for (auto& slot : slotElements) {
        slot->render();
    }
}

bool UIInventoryGrid::onClick(const Vec2& pos) {
    if (!isVisible()) return false;
    
    Vec2 gridPos = getPosition();
    Vec2 gridSize = getSize();
    
    if (pos.x >= gridPos.x && pos.x <= gridPos.x + gridSize.x &&
        pos.y >= gridPos.y && pos.y <= gridPos.y + gridSize.y) {
        
        // Check individual slots
        for (auto& slot : slotElements) {
            if (slot->onClick(pos)) {
                return true;
            }
        }
        
        return true;
    }
    
    return false;
}

// ==================== UIDraggedItem ====================

UIDraggedItem::UIDraggedItem() : visible(false), offset(Vec2(-25, -25)) {
    setSize(Vec2(50, 50));
}

void UIDraggedItem::setItem(const ItemInstance& itemInstance) {
    item = itemInstance;
    visible = true;
}

void UIDraggedItem::clearItem() {
    item = ItemInstance();
    visible = false;
}

void UIDraggedItem::updatePosition(const Vec2& mousePos) {
    setPosition(Vec2(mousePos.x + offset.x, mousePos.y + offset.y));
}

void UIDraggedItem::render() {
    if (!visible || !isVisible()) return;
    
    // TODO: Render dragged item icon
    // This is a placeholder
}

// ==================== UIInventoryTooltip ====================

UIInventoryTooltip::UIInventoryTooltip()
    : visible(false), backgroundColor(0x222222EE), textColor(0xFFFFFFFF), maxWidth(200.0f) {
    setSize(Vec2(200, 100));
}

void UIInventoryTooltip::show(const std::string& item, const Vec2& pos) {
    itemID = item;
    visible = true;
    setPosition(pos);
    
    // Generate tooltip text
    tooltipText = InventoryTooltip::GenerateTooltipText(item);
    
    // Adjust size based on text
    // TODO: Calculate proper size based on text content
}

void UIInventoryTooltip::hide() {
    visible = false;
    itemID.clear();
    tooltipText.clear();
}

void UIInventoryTooltip::render() {
    if (!visible || !isVisible()) return;
    
    // TODO: Render tooltip background and text
    // This is a placeholder
}

// ==================== UIInventoryController ====================

UIInventoryController::UIInventoryController(InventoryManager* manager)
    : inventoryManager(manager), isInventoryOpen(false), isDragging(false),
      draggedFromSlot(-1), currentFilter(ItemCategory::Misc) {
}

bool UIInventoryController::initialize() {
    if (!inventoryManager) return false;
    
    // Create player inventory grid (9x4 = 36 slots: 9 hotbar + 27 inventory)
    playerInventoryGrid = std::make_shared<UIInventoryGrid>("player_inventory", 9, 4);
    playerInventoryGrid->setPosition(Vec2(100, 200));
    playerInventoryGrid->setSlotSize(50.0f);
    playerInventoryGrid->setSlotSpacing(5.0f);
    
    // Create hotbar grid (9x1)
    hotbarGrid = std::make_shared<UIInventoryGrid>("player_hotbar", 9, 1);
    hotbarGrid->setPosition(Vec2(100, 500));
    hotbarGrid->setSlotSize(50.0f);
    hotbarGrid->setSlotSpacing(5.0f);
    
    // Create dragged item visual
    draggedItem = std::make_shared<UIDraggedItem>();
    draggedItem->setDragVisible(false);
    
    // Create tooltip
    tooltip = std::make_shared<UIInventoryTooltip>();
    tooltip->hide();
    
    // Set up callbacks
    playerInventoryGrid->onSlotClicked = [this](int slot) {
        handleMouseClick(playerInventoryGrid->getSlotPosition(slot), false);
    };
    
    hotbarGrid->onSlotClicked = [this](int slot) {
        handleMouseClick(hotbarGrid->getSlotPosition(slot), false);
    };
    
    Logger::Info("[UIInventoryController] Initialized");
    return true;
}

void UIInventoryController::shutdown() {
    playerInventoryGrid.reset();
    hotbarGrid.reset();
    draggedItem.reset();
    tooltip.reset();
    isInventoryOpen = false;
    isDragging = false;
}

void UIInventoryController::openInventory() {
    isInventoryOpen = true;
    Logger::Info("[UIInventoryController] Inventory opened");
}

void UIInventoryController::closeInventory() {
    isInventoryOpen = false;
    
    // Cancel any ongoing drag
    if (isDragging) {
        inventoryManager->CancelDrag();
        isDragging = false;
        draggedItem->clearItem();
    }
    
    tooltip->hide();
    Logger::Info("[UIInventoryController] Inventory closed");
}

void UIInventoryController::toggleInventory() {
    if (isInventoryOpen) {
        closeInventory();
    } else {
        openInventory();
    }
}

void UIInventoryTooltip::Update(float deltaTime) {
    (void)deltaTime;
    // Timer-based visibility logic could go here
}

void UIInventoryController::update(float deltaTime) {
    if (!isInventoryOpen) return;
    
    // Update tooltip timer
    tooltip->Update(deltaTime);
}

void UIInventoryController::updateDrag(const Vec2& mousePos) {
    if (isDragging) {
        draggedItem->updatePosition(mousePos);
        inventoryManager->UpdateDragPosition(mousePos);
    }
}

void UIInventoryController::updateTooltip(const Vec2& mousePos, int hoveredSlot) {
    if (hoveredSlot >= 0 && isInventoryOpen) {
        Inventory* inv = inventoryManager->GetActiveInventory();
        if (inv && hoveredSlot < inv->GetTotalSlots()) {
            const InventorySlot& slot = inv->GetSlot(hoveredSlot);
            if (!slot.IsEmpty()) {
                tooltip->show(slot.item.itemID, mousePos);
                return;
            }
        }
    }
    
    tooltip->hide();
}

bool UIInventoryController::handleMouseClick(const Vec2& pos, bool isRightClick) {
    if (!isInventoryOpen) return false;
    
    // Check which grid was clicked
    int slot = -1;
    std::string invID;
    
    slot = playerInventoryGrid->getSlotAtPosition(pos);
    if (slot >= 0) {
        invID = "player_inventory";
    } else {
        slot = hotbarGrid->getSlotAtPosition(pos);
        if (slot >= 0) {
            invID = "player_hotbar";
        }
    }
    
    if (slot < 0) return false;
    
    if (isRightClick) {
        // Right click - split stack or use item
        Inventory* inv = inventoryManager->GetInventory(invID);
        if (inv) {
            inv->SplitStack(slot, 1);
        }
        return true;
    }
    
    if (isDragging) {
        // End drag
        bool success = inventoryManager->EndDrag(invID, slot);
        if (success) {
            isDragging = false;
            draggedItem->clearItem();
        } else {
            // Cancel drag if drop failed
            inventoryManager->CancelDrag();
            isDragging = false;
            draggedItem->clearItem();
        }
        return true;
    } else {
        // Start drag
        bool success = inventoryManager->StartDrag(invID, slot);
        if (success) {
            isDragging = true;
            draggedFromSlot = slot;
            draggedFromInventory = invID;
            draggedItem->setItem(inventoryManager->GetDraggedItem());
            draggedItem->setDragVisible(true);
        }
        return true;
    }
    
    return false;
}

bool UIInventoryController::handleMouseRelease(const Vec2& pos) {
    if (!isDragging) return false;
    
    // Try to drop at current position
    int slot = -1;
    std::string invID;
    
    slot = playerInventoryGrid->getSlotAtPosition(pos);
    if (slot >= 0) {
        invID = "player_inventory";
    } else {
        slot = hotbarGrid->getSlotAtPosition(pos);
        if (slot >= 0) {
            invID = "player_hotbar";
        }
    }
    
    if (slot >= 0) {
        bool success = inventoryManager->EndDrag(invID, slot);
        if (success) {
            isDragging = false;
            draggedItem->clearItem();
            return true;
        }
    }
    
    // Drop to world if not over inventory
    inventoryManager->EndDragToWorld();
    isDragging = false;
    draggedItem->clearItem();
    return true;
}

bool UIInventoryController::handleMouseMove(const Vec2& pos) {
    if (!isInventoryOpen) return false;
    
    // Update drag position
    if (isDragging) {
        updateDrag(pos);
    }
    
    // Update hover states
    int hoveredSlot = playerInventoryGrid->getSlotAtPosition(pos);
    if (hoveredSlot >= 0) {
        auto slot = playerInventoryGrid->getSlotElement(hoveredSlot);
        if (slot) {
            slot->setHovered(true);
        }
        updateTooltip(pos, hoveredSlot);
        return true;
    }
    
    hoveredSlot = hotbarGrid->getSlotAtPosition(pos);
    if (hoveredSlot >= 0) {
        auto slot = hotbarGrid->getSlotElement(hoveredSlot);
        if (slot) {
            slot->setHovered(true);
        }
        updateTooltip(pos, hoveredSlot);
        return true;
    }
    
    // Not hovering any slot
    tooltip->hide();
    return false;
}

bool UIInventoryController::handleKeyPress(int key) {
    // Handle inventory key (typically 'E' or 'I')
    if (key == 'E' || key == 'e' || key == 'I' || key == 'i') {
        toggleInventory();
        return true;
    }
    
    // Handle number keys for hotbar selection
    if (key >= '1' && key <= '9') {
        int slot = key - '1';
        // TODO: Set hotbar selection
        return true;
    }
    
    return false;
}

void UIInventoryController::setCategoryFilter(ItemCategory category) {
    currentFilter = category;
    
    Inventory* inv = inventoryManager->GetActiveInventory();
    if (inv) {
        inv->SetFilter(category);
    }
    
    // Refresh grid display
    playerInventoryGrid->refreshSlots();
    Logger::Info("[UIInventoryController] Filter set to category: " + std::to_string((int)category));
}

void UIInventoryController::clearCategoryFilter() {
    currentFilter = ItemCategory::Misc;
    
    Inventory* inv = inventoryManager->GetActiveInventory();
    if (inv) {
        inv->ClearFilter();
    }
    
    playerInventoryGrid->refreshSlots();
    Logger::Info("[UIInventoryController] Category filter cleared");
}

void UIInventoryController::render() {
    if (!isInventoryOpen) return;
    
    // Render inventory grids
    playerInventoryGrid->render();
    hotbarGrid->render();
    
    // Render dragged item on top
    if (isDragging) {
        draggedItem->render();
    }
    
    // Render tooltip on top
    if (tooltip->isTooltipVisible()) {
        tooltip->render();
    }
}

// ==================== UICraftingPanel ====================

UICraftingPanel::UICraftingPanel(InventoryManager* manager)
    : inventoryManager(manager) {
}

bool UICraftingPanel::initialize() {
    if (!inventoryManager) return false;
    
    // Create 3x3 crafting grid
    craftingGrid = std::make_shared<UIInventoryGrid>("crafting_grid", 3, 3);
    craftingGrid->setPosition(Vec2(500, 200));
    craftingGrid->setSlotSize(50.0f);
    craftingGrid->setSlotSpacing(5.0f);
    
    // Create result slot
    resultSlot = std::make_shared<UIInventorySlot>(-1, "crafting_result");
    resultSlot->setPosition(Vec2(700, 275));
    resultSlot->setSize(Vec2(50, 50));
    
    Logger::Info("[UICraftingPanel] Initialized");
    return true;
}

void UICraftingPanel::refreshRecipes() {
    // TODO: Load available recipes based on inventory contents
}

void UICraftingPanel::tryCraft() {
    // TODO: Implement crafting logic using CraftingSystem
    Logger::Info("[UICraftingPanel] Crafting attempt");
}

void UICraftingPanel::onCraftingSlotChanged(int slot) {
    // TODO: Check if current grid matches any recipe
    refreshRecipes();
}

void UICraftingPanel::onCraftingResultTaken() {
    // TODO: Consume ingredients and clear crafting grid
    Logger::Info("[UICraftingPanel] Result taken");
}

void UICraftingPanel::render() {
    if (!isVisible()) return;
    
    craftingGrid->render();
    resultSlot->render();
}

} // namespace vge
