#include "tool_system.h"
#include <algorithm>

namespace vge {

ToolSystem::ToolSystem() {}

ToolSystem::~ToolSystem() {}

ToolStats ToolSystem::CreateTool(ToolType type, ToolMaterial material) {
    ToolStats stats;
    stats.type = type;
    stats.material = material;
    stats.damage = GetBaseDamage(type, material);
    stats.miningSpeed = GetBaseMiningSpeed(material);
    stats.durability = GetBaseDurability(material);
    stats.currentDurability = stats.durability;
    stats.isTool = (type != ToolType::None);
    stats.name = GetToolName(type, material);
    return stats;
}

float ToolSystem::GetMiningSpeed(const ToolStats& tool, const std::string& blockId) {
    if (!tool.isTool || tool.currentDurability <= 0) return 1.0f;
    
    // Check if tool is appropriate for block
    bool appropriate = false;
    switch (tool.type) {
        case ToolType::Pickaxe:
            appropriate = (blockId == "stone" || blockId == "coal_ore" || 
                          blockId == "iron_ore" || blockId == "gold_ore" || 
                          blockId == "diamond_ore" || blockId == "emerald_ore" ||
                          blockId == "bedrock");
            break;
        case ToolType::Axe:
            appropriate = (blockId == "wood" || blockId == "leaves");
            break;
        case ToolType::Shovel:
            appropriate = (blockId == "dirt" || blockId == "grass" || blockId == "sand");
            break;
        case ToolType::Sword:
            appropriate = false; // Swords aren't for mining
            break;
        default:
            break;
    }
    
    if (!appropriate) return 1.0f; // Bare hand speed
    
    return tool.miningSpeed;
}

bool ToolSystem::CanMine(const ToolStats& tool, const std::string& blockId) {
    if (!tool.isTool || tool.currentDurability <= 0) {
        // Bare hands can only mine certain blocks
        return (blockId == "dirt" || blockId == "grass" || blockId == "sand" || 
                blockId == "leaves" || blockId == "flower" || blockId == "tall_grass");
    }
    
    // Check material tier requirements
    int toolTier = GetMaterialTier(tool.material);
    
    if (blockId == "bedrock") return false; // Unbreakable
    if (blockId == "diamond_ore" || blockId == "emerald_ore") return toolTier >= 2; // Need iron+
    if (blockId == "iron_ore") return toolTier >= 1; // Need stone+
    if (blockId == "gold_ore") return toolTier >= 2; // Need iron+
    
    return true;
}

float ToolSystem::GetDamage(const ToolStats& tool) {
    if (!tool.isTool || tool.currentDurability <= 0) return 1.0f; // Bare hand
    return tool.damage;
}

bool ToolSystem::UseTool(ToolStats& tool) {
    if (!tool.isTool || tool.currentDurability <= 0) return false;
    
    tool.currentDurability--;
    return tool.currentDurability > 0;
}

void ToolSystem::RepairTool(ToolStats& tool, int amount) {
    if (!tool.isTool) return;
    tool.currentDurability = std::min(tool.durability, tool.currentDurability + amount);
}

std::string ToolSystem::GetToolName(ToolType type, ToolMaterial material) {
    std::string materialName;
    switch (material) {
        case ToolMaterial::Wood: materialName = "Wooden"; break;
        case ToolMaterial::Stone: materialName = "Stone"; break;
        case ToolMaterial::Iron: materialName = "Iron"; break;
        case ToolMaterial::Gold: materialName = "Golden"; break;
        case ToolMaterial::Diamond: materialName = "Diamond"; break;
    }
    
    std::string typeName;
    switch (type) {
        case ToolType::Pickaxe: typeName = "Pickaxe"; break;
        case ToolType::Axe: typeName = "Axe"; break;
        case ToolType::Shovel: typeName = "Shovel"; break;
        case ToolType::Sword: typeName = "Sword"; break;
        case ToolType::Hoe: typeName = "Hoe"; break;
        default: typeName = "Tool"; break;
    }
    
    return materialName + " " + typeName;
}

int ToolSystem::GetMaterialTier(ToolMaterial material) {
    switch (material) {
        case ToolMaterial::Wood: return 0;
        case ToolMaterial::Stone: return 1;
        case ToolMaterial::Iron: return 2;
        case ToolMaterial::Gold: return 0; // Gold is weak despite being rare
        case ToolMaterial::Diamond: return 3;
    }
    return 0;
}

float ToolSystem::GetBaseMiningSpeed(ToolMaterial material) {
    switch (material) {
        case ToolMaterial::Wood: return 2.0f;
        case ToolMaterial::Stone: return 4.0f;
        case ToolMaterial::Iron: return 6.0f;
        case ToolMaterial::Gold: return 12.0f; // Very fast but weak
        case ToolMaterial::Diamond: return 8.0f;
    }
    return 1.0f;
}

int ToolSystem::GetBaseDurability(ToolMaterial material) {
    switch (material) {
        case ToolMaterial::Wood: return 60;
        case ToolMaterial::Stone: return 132;
        case ToolMaterial::Iron: return 251;
        case ToolMaterial::Gold: return 33; // Very fragile
        case ToolMaterial::Diamond: return 1562;
    }
    return 1;
}

float ToolSystem::GetBaseDamage(ToolType type, ToolMaterial material) {
    float baseDamage = 1.0f;
    
    // Material multiplier
    switch (material) {
        case ToolMaterial::Wood: baseDamage = 2.0f; break;
        case ToolMaterial::Stone: baseDamage = 3.0f; break;
        case ToolMaterial::Iron: baseDamage = 4.0f; break;
        case ToolMaterial::Gold: baseDamage = 2.0f; break;
        case ToolMaterial::Diamond: baseDamage = 5.0f; break;
    }
    
    // Tool type modifier
    switch (type) {
        case ToolType::Sword: baseDamage *= 1.5f; break; // Swords do more damage
        case ToolType::Axe: baseDamage *= 1.2f; break;
        default: break;
    }
    
    return baseDamage;
}

} // namespace vge
