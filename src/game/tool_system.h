#pragma once
#include <string>
#include <functional>

namespace vge {

// ============================================
// Tool Types
// ============================================
enum class ToolType {
    None,
    Pickaxe,    // Mines stone, ores
    Axe,        // Chops wood
    Shovel,     // Digs dirt, sand, gravel
    Sword,      // Combat
    Hoe         // Farming
};

// ============================================
// Tool Material/Tier
// ============================================
enum class ToolMaterial {
    Wood,       // Lowest tier
    Stone,
    Iron,
    Gold,       // Fast but weak
    Diamond     // Best tier
};

// ============================================
// Tool Stats
// ============================================
struct ToolStats {
    ToolType type;
    ToolMaterial material;
    std::string name;
    float damage;           // Attack damage
    float miningSpeed;      // Mining speed multiplier
    int durability;         // Max uses
    int currentDurability;  // Remaining uses
    bool isTool;
    
    ToolStats()
        : type(ToolType::None)
        , material(ToolMaterial::Wood)
        , damage(1.0f)
        , miningSpeed(1.0f)
        , durability(60)
        , currentDurability(60)
        , isTool(false) {}
};

// ============================================
// Tool System
// ============================================
class ToolSystem {
public:
    ToolSystem();
    ~ToolSystem();

    // Create tool stats
    static ToolStats CreateTool(ToolType type, ToolMaterial material);
    
    // Get mining speed for block
    static float GetMiningSpeed(const ToolStats& tool, const std::string& blockId);
    
    // Check if tool can mine block
    static bool CanMine(const ToolStats& tool, const std::string& blockId);
    
    // Get damage for entity
    static float GetDamage(const ToolStats& tool);
    
    // Use tool (reduce durability)
    static bool UseTool(ToolStats& tool);
    
    // Repair tool
    static void RepairTool(ToolStats& tool, int amount);
    
    // Get tool name
    static std::string GetToolName(ToolType type, ToolMaterial material);
    
    // Material tier order
    static int GetMaterialTier(ToolMaterial material);

private:
    static float GetBaseMiningSpeed(ToolMaterial material);
    static int GetBaseDurability(ToolMaterial material);
    static float GetBaseDamage(ToolType type, ToolMaterial material);
};

} // namespace vge
