# Tool System

## Quick Reference

```cpp
// Create a diamond pickaxe
vge::ToolStats pickaxe = vge::ToolSystem::CreateTool(
    vge::ToolType::Pickaxe, 
    vge::ToolMaterial::Diamond
);

// Check if can mine block
if (vge::ToolSystem::CanMine(pickaxe, "diamond_ore")) {
    float speed = vge::ToolSystem::GetMiningSpeed(pickaxe, "diamond_ore");
    // Mine block...
    vge::ToolSystem::UseTool(pickaxe); // Reduce durability
}

// Combat damage
float damage = vge::ToolSystem::GetDamage(pickaxe);
```

## Tool Types

| Type | Use | Blocks |
|------|-----|--------|
| Pickaxe | Mining stone/ores | Stone, coal, iron, gold, diamond, emerald, bedrock |
| Axe | Chopping wood | Wood, leaves |
| Shovel | Digging soft blocks | Dirt, grass, sand |
| Sword | Combat | Not for mining |
| Hoe | Farming | Soil |

## Material Tiers

| Material | Mining Speed | Durability | Damage | Tier |
|----------|-------------|------------|--------|------|
| Wood | 2x | 60 | 2 | 0 |
| Stone | 4x | 132 | 3 | 1 |
| Iron | 6x | 251 | 4 | 2 |
| Gold | 12x | 33 | 2 | 0 |
| Diamond | 8x | 1562 | 5 | 3 |

## Block Requirements

| Block | Required Tool | Minimum Tier |
|-------|--------------|--------------|
| Stone | Pickaxe | Wood (0) |
| Coal Ore | Pickaxe | Wood (0) |
| Iron Ore | Pickaxe | Stone (1) |
| Gold Ore | Pickaxe | Iron (2) |
| Diamond Ore | Pickaxe | Iron (2) |
| Emerald Ore | Pickaxe | Iron (2) |
| Bedrock | Unbreakable | - |

## Usage

```cpp
// Create tools
auto woodPickaxe = vge::ToolSystem::CreateTool(ToolType::Pickaxe, ToolMaterial::Wood);
auto ironSword = vge::ToolSystem::CreateTool(ToolType::Sword, ToolMaterial::Iron);

// Mining
std::string blockId = "iron_ore";
if (vge::ToolSystem::CanMine(woodPickaxe, blockId)) {
    // Can mine but slowly (1x speed, no bonus)
}
if (vge::ToolSystem::CanMine(ironPickaxe, blockId)) {
    // Can mine at 6x speed
}

// Durability
bool stillUsable = vge::ToolSystem::UseTool(pickaxe); // Returns false if broken

// Repair
vge::ToolSystem::RepairTool(pickaxe, 50); // Add 50 durability
```

## Files
- `src/game/tool_system.h`
- `src/game/tool_system.cpp`
