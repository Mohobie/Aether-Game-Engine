# Aether Game Engine - TODO List

## JSON Serialization (8 items)
These require a JSON library (e.g., nlohmann/json) to be added to the project.

- [x] **src/ai/entity_ai.cpp:69** - `LoadFromFile()` - Load entity archetypes from JSON files ✅
- [x] **src/ai/entity_ai.cpp:75** - `LoadFromDirectory()` - Batch load archetype JSON files from directory ✅
- [x] **src/audio/audio_events.cpp:180** - `LoadFromFile()` - Load sound pack definitions from JSON ✅
- [x] **src/audio/audio_events.cpp:186** - `SaveToFile()` - Save sound pack definitions to JSON ✅
- [x] **src/core/achievements.cpp:213** - `LoadFromFile()` - Load achievement definitions from JSON ✅
- [x] **src/core/achievements.cpp:219** - `SaveToFile()` - Save achievement definitions to JSON ✅
- [ ] **src/core/item_system.cpp:50** - `LoadFromFile()` - Load item definitions from JSON
- [ ] **src/core/item_system.cpp:56** - `LoadFromJson()` - Parse item JSON data
- [ ] **src/core/item_system.cpp:395** - `Deserialize()` - Deserialize item data from JSON
- [ ] **src/core/item_system.cpp:661** - `Deserialize()` - Deserialize inventory from JSON

## UI Implementation (8 items)
These need actual rendering code for the inventory/crafting UI.

- [x] **src/ui/ui_inventory.cpp:38** - Render slot background, item icon, and count text ✅
- [x] **src/ui/ui_inventory.cpp:114** - Integrate with actual inventory data ✅
- [x] **src/ui/ui_inventory.cpp:163** - Actual rendering implementation for inventory grid ✅
- [x] **src/ui/ui_inventory.cpp:216** - Render dragged item icon during drag-and-drop ✅
- [x] **src/ui/ui_inventory.cpp:236** - Calculate proper tooltip size based on text content ✅
- [x] **src/ui/ui_inventory.cpp:248** - Render tooltip background and text ✅
- [x] **src/ui/ui_inventory.cpp:499** - Set hotbar selection on click ✅
- [x] **src/ui/ui_inventory.cpp:574** - Load available recipes based on inventory contents ✅
- [x] **src/ui/ui_inventory.cpp:578** - Implement crafting logic using CraftingSystem ✅
- [x] **src/ui/ui_inventory.cpp:583** - Check if current grid matches any recipe ✅
- [x] **src/ui/ui_inventory.cpp:588** - Consume ingredients and clear crafting grid ✅
- [x] **src/ui/inventory_ui.cpp:793** - Handle crafting grid drops ✅

## Game Logic (3 items)

- [x] **src/core/crafting_inventory_bridge.cpp:274** - Map block types to item categories ✅
- [ ] **src/core/item_system.cpp:468** - Use proper inventory index instead of hardcoded 0
- [x] **src/voxel/chunk_manager.cpp:84** - Save chunks to disk (chunk persistence) ✅

## AI (1 item)

- [x] **src/ai/entity_ai.cpp:100** - Check if entity is in sunlight (for sunlight damage) ✅

---

## Priority Order

### High Priority (Core Functionality)
1. JSON serialization for entity archetypes, items, achievements
2. Inventory UI rendering integration
3. Crafting logic implementation
4. Chunk save/load to disk

### Medium Priority (Polish)
5. UI tooltip rendering
6. Drag-and-drop visual feedback
7. Hotbar selection
8. Block type to item category mapping

### Low Priority (Nice to Have)
9. Audio events JSON serialization
10. Inventory index fix
11. Sunlight detection for AI
