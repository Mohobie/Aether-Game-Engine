# Tool System

## Quick Reference

```cpp
// Create a tool
vge::ToolStats pickaxe = vge::ToolSystem::CreateTool(vge::ToolType::Pickaxe, vge::ToolMaterial::Iron);

// Use tool (reduces durability)
bool stillUsable = vge::ToolSystem::UseTool(pickaxe);

// Check mining speed
float speed = vge::ToolSystem::GetMiningSpeed(pickaxe, "stone");

// Check if can mine
bool canMine = vge::ToolSystem::CanMine(pickaxe, "diamond_ore");

// Get damage
float damage = vge::ToolSystem::GetDamage(pickaxe);

// Repair tool
vge::ToolSystem::RepairTool(pickaxe, 50);
```

## Features

### Tool Types
| Type | Use | Blocks |
|------|-----|--------|
| Pickaxe | Mining stone/ores | Stone, coal, iron, gold, diamond |
| Axe | Chopping wood | Wood, leaves |
| Shovel | Digging | Dirt, grass, sand |
| Sword | Combat | Not for mining |
| Hoe | Farming | - |

### Materials
| Material | Tier | Durability | Speed | Damage |
|----------|------|------------|-------|--------|
| Wood | 0 | 60 | 2x | 2 |
| Stone | 1 | 132 | 4x | 3 |
| Iron | 2 | 251 | 6x | 4 |
| Gold | 0 | 33 | 12x | 2 |
| Diamond | 3 | 1562 | 8x | 5 |

### Mining Requirements
| Block | Required Tool | Min Tier |
|-------|--------------|----------|
| Stone | Pickaxe | Wood (0) |
| Coal | Pickaxe | Wood (0) |
| Iron Ore | Pickaxe | Stone (1) |
| Gold Ore | Pickaxe | Iron (2) |
| Diamond Ore | Pickaxe | Iron (2) |
| Emerald Ore | Pickaxe | Iron (2) |
| Bedrock | - | Unbreakable |

## Implementation

```cpp
// Create tools
vge::ToolStats woodenPick = vge::ToolSystem::CreateTool(vge::ToolType::Pickaxe, vge::ToolMaterial::Wood);
vge::ToolStats ironSword = vge::ToolSystem::CreateTool(vge::ToolType::Sword, vge::ToolMaterial::Iron);

// Use in game loop
if (mining) {
    float speed = vge::ToolSystem::GetMiningSpeed(tool, blockId);
    if (vge::ToolSystem::CanMine(tool, blockId)) {
        // Mine block
        vge::ToolSystem::UseTool(tool);
    }
}

// Combat
if (attacking) {
    float damage = vge::ToolSystem::GetDamage(weapon);
    enemy.TakeDamage(damage);
    vge::ToolSystem::UseTool(weapon);
}
```

## Files
- `src/game/tool_system.h`
- `src/game/tool_system.cpp`
