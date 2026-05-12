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
    setSize(aether::Vec2(50, 50));
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
    
    // Get position and size
    aether::Vec2 pos = getPosition();
    aether::Vec2 size = getSize();
    
    // Note: Actual rendering would use the engine's 2D rendering system
    // For now, this is a placeholder that logs rendering info
    // In a real implementation, this would call:
    // - Renderer2D::DrawRect() for background
    // - TextureManager::Draw() for item icon
    // - FontRenderer::DrawText() for count
    
    (void)color; // Suppress unused warning
    (void)pos;
    (void)size;
}

bool UIInventorySlot::onClick(const aether::Vec2& pos) {
    if (!isVisible()) return false;
    
    aether::Vec2 slotPos = getPosition();
    aether::Vec2 slotSize = getSize();
    
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
    setSize(aether::Vec2(totalWidth, totalHeight));
    
    // Create slot elements
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = y * width + x;
            auto slot = std::make_shared<UIInventorySlot>(index, invID);
            
            float posX = x * (slotSize + slotSpacing);
            float posY = y * (slotSize + slotSpacing);
            slot->setPosition(aether::Vec2(posX, posY));
            slot->setSize(aether::Vec2(slotSize, slotSize));
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
    // This would refresh item icons, counts, etc.
    // For now, this is a placeholder
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

int UIInventoryGrid::getSlotAtPosition(const aether::Vec2& pos) const {
    aether::Vec2 gridPos = getPosition();
    aether::Vec2 localPos = aether::Vec2(pos.x - gridPos.x, pos.y - gridPos.y);
    
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

aether::Vec2 UIInventoryGrid::getSlotPosition(int index) const {
    int x = index % gridWidth;
    int y = index / gridWidth;
    
    aether::Vec2 gridPos = getPosition();
    float slotX = gridPos.x + x * (slotSize + slotSpacing);
    float slotY = gridPos.y + y * (slotSize + slotSpacing);
    
    return aether::Vec2(slotX, slotY);
}

void UIInventoryGrid::render() {
    if (!isVisible()) return;
    
    // Render grid background
    // Note: Actual rendering would use the engine's 2D rendering system
    
    // Render slots
    for (auto& slot : slotElements) {
        slot->render();
    }
}

bool UIInventoryGrid::onClick(const aether::Vec2& pos) {
    if (!isVisible()) return false;
    
    aether::Vec2 gridPos = getPosition();
    aether::Vec2 gridSize = getSize();
    
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

UIDraggedItem::UIDraggedItem() : visible(false), offset(aether::Vec2(-25, -25)) {
    setSize(aether::Vec2(50, 50));
}

void UIDraggedItem::setItem(const ItemInstance& itemInstance) {
    item = itemInstance;
    visible = true;
}

void UIDraggedItem::clearItem() {
    item = ItemInstance();
    visible = false;
}

void UIDraggedItem::updatePosition(const aether::Vec2& mousePos) {
    setPosition(aether::Vec2(mousePos.x + offset.x, mousePos.y + offset.y));
}

void UIDraggedItem::render() {
    if (!visible || !isVisible()) return;
    
    // Note: Actual rendering would draw the dragged item icon
    // with transparency at the current mouse position
}

// ==================== UIInventoryTooltip ====================

UIInventoryTooltip::UIInventoryTooltip()
    : visible(false), backgroundColor(0x222222EE), textColor(0xFFFFFFFF), maxWidth(200.0f) {
    setSize(aether::Vec2(200, 100));
}

void UIInventoryTooltip::show(const std::string& item, const aether::Vec2& pos) {
    itemID = item;
    visible = true;
    setPosition(pos);
    
    // Generate tooltip text
    tooltipText = InventoryTooltip::GenerateTooltipText(item);
    
    // Calculate proper size based on text content
    int lineCount = 1;
    float textWidth = 0;
    float lineHeight = 18.0f;
    
    for (char c : tooltipText) {
        if (c == '\n') {
            lineCount++;
            textWidth = 0;
        } else {
            textWidth += 8.0f; // Approximate char width
        }
    }
    
    float width = std::min(textWidth + 20.0f, maxWidth);
    float height = lineCount * lineHeight + 20.0f;
    setSize(aether::Vec2(width, height));
}

void UIInventoryTooltip::hide() {
    visible = false;
    itemID.clear();
    tooltipText.clear();
}

void UIInventoryTooltip::render() {
    if (!visible || !isVisible()) return;
    
    // Note: Actual rendering would draw tooltip background and text
    // using the engine's 2D rendering system
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
    playerInventoryGrid->setPosition(aether::Vec2(100, 200));
    playerInventoryGrid->setSlotSize(50.0f);
    playerInventoryGrid->setSlotSpacing(5.0f);
    
    // Create hotbar grid (9x1)
    hotbarGrid = std::make_shared<UIInventoryGrid>("player_hotbar", 9, 1);
    hotbarGrid->setPosition(aether::Vec2(100, 500));
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

void UIInventoryController::updateDrag(const aether::Vec2& mousePos) {
    if (isDragging) {
        draggedItem->updatePosition(mousePos);
        inventoryManager->UpdateDragPosition(mousePos);
    }
}

void UIInventoryController::updateTooltip(const aether::Vec2& mousePos, int hoveredSlot) {
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

bool UIInventoryController::handleMouseClick(const aether::Vec2& pos, bool isRightClick) {
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

bool UIInventoryController::handleMouseRelease(const aether::Vec2& pos) {
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

bool UIInventoryController::handleMouseMove(const aether::Vec2& pos) {
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
        // Set hotbar selection
        Inventory* hotbar = inventoryManager->GetInventory("player_hotbar");
        if (hotbar && slot < hotbar->GetTotalSlots()) {
            Logger::Info("[UIInventoryController] Hotbar slot selected: " + std::to_string(slot));
        }
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
    craftingGrid->setPosition(aether::Vec2(500, 200));
    craftingGrid->setSlotSize(50.0f);
    craftingGrid->setSlotSpacing(5.0f);
    
    // Create result slot
    resultSlot = std::make_shared<UIInventorySlot>(-1, "crafting_result");
    resultSlot->setPosition(aether::Vec2(700, 275));
    resultSlot->setSize(aether::Vec2(50, 50));
    
    Logger::Info("[UICraftingPanel] Initialized");
    return true;
}

void UICraftingPanel::refreshRecipes() {
    // Load available recipes based on inventory contents
    availableRecipes.clear();
    
    Inventory* inv = inventoryManager->GetInventory("player_inventory");
    if (!inv) return;
    
    // Get all recipes from crafting system
    CraftingSystem craftingSystem;
    auto allRecipes = craftingSystem.GetAllRecipes();
    
    // Filter recipes that can be crafted with current inventory
    for (const auto* recipe : allRecipes) {
        if (recipe && recipe->MatchesInventory(*inv)) {
            availableRecipes.push_back(*recipe);
        }
    }
    
    Logger::Info("[UICraftingPanel] Found " + std::to_string(availableRecipes.size()) + " craftable recipes");
}

void UICraftingPanel::tryCraft() {
    // Get crafting grid contents
    Inventory* grid = inventoryManager->GetInventory("crafting_grid");
    Inventory* playerInv = inventoryManager->GetInventory("player_inventory");
    
    if (!grid || !playerInv) return;
    
    // Check if grid matches any recipe
    CraftingSystem craftingSystem;
    CraftingGrid craftGrid;
    
    // Convert inventory to crafting grid
    for (int i = 0; i < grid->GetTotalSlots() && i < 9; ++i) {
        const InventorySlot& slot = grid->GetSlot(i);
        if (!slot.IsEmpty()) {
            int x = i % 3;
            int y = i / 3;
            craftGrid.SetItem(x, y, slot.item.itemID);
        }
    }
    
    const CraftingRecipe* matchedRecipe = craftingSystem.FindRecipe(craftGrid);
    
    if (matchedRecipe) {
        // Try to craft
        CraftingResult2 result = craftingSystem.TryCraft(craftGrid, playerInv);
        if (result.success) {
            // Clear crafting grid
            for (int i = 0; i < grid->GetTotalSlots(); ++i) {
                grid->ClearSlot(i);
            }
            Logger::Info("[UICraftingPanel] Crafted: " + result.recipeID);
        }
    } else {
        Logger::Info("[UICraftingPanel] No matching recipe");
    }
    

}

void UICraftingPanel::onCraftingSlotChanged(int slot) {
    // Check if current grid matches any recipe
    refreshRecipes();
    
    // Update result slot if recipe matches
    if (!availableRecipes.empty()) {
        const auto& recipe = availableRecipes[0];
        // Could update result slot visual here
    }
}

void UICraftingPanel::onCraftingResultTaken() {
    // Consume ingredients and clear crafting grid
    Inventory* grid = inventoryManager->GetInventory("crafting_grid");
    if (!grid) return;
    
    // Clear all crafting slots
    for (int i = 0; i < grid->GetTotalSlots(); ++i) {
        grid->ClearSlot(i);
    }
    
    Logger::Info("[UICraftingPanel] Result taken, grid cleared");
}

void UICraftingPanel::render() {
    if (!isVisible()) return;
    
    craftingGrid->render();
    resultSlot->render();
}

} // namespace vge
