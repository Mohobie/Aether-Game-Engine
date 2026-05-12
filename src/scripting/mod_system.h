#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <unordered_map>

namespace vge {

// Forward declarations
class ScriptEngine;
class World;
class BlockRegistry;
class EntityManager;
class CraftingSystem;
class AudioSystem;

// ============================================
// Mod Manifest
// ============================================
struct ModManifest {
    std::string name;
    std::string version;
    std::string author;
    std::string description;
    std::string entryPoint;      // e.g., "main.lua"
    std::vector<std::string> dependencies;
    std::vector<std::string> optionalDependencies;
    bool enabled;

    ModManifest()
        : enabled(true)
    {}

    bool LoadFromJson(const std::string& json);
    bool LoadFromFile(const std::string& path);
    std::string ToJson() const;
};

// ============================================
// Mod Definition
// ============================================
struct ModDef {
    ModManifest manifest;
    std::string directory;
    bool loaded;
    bool hasErrors;
    std::string errorMessage;

    ModDef() : loaded(false), hasErrors(false) {}
};

// ============================================
// Mod Registration API
// ============================================
struct BlockRegistration {
    std::string id;
    std::string name;
    bool solid;
    bool opaque;
    float hardness;
    std::string color;
    int emission;
    std::string texture;
};

struct EntityRegistration {
    std::string id;
    std::string name;
    float health;
    float speed;
    std::string model;
};

struct ItemRegistration {
    std::string id;
    std::string name;
    std::string description;
    int maxStack;
    bool consumable;
};

struct RecipeRegistration {
    std::string recipeID;
    std::string outputItemID;
    int outputCount;
    bool shapeless;
    std::vector<std::pair<std::string, int>> ingredients;
};

struct BiomeRegistration {
    std::string id;
    std::string name;
    float temperature;
    float humidity;
    std::vector<std::string> blocks;
};

struct SoundRegistration {
    std::string id;
    std::string path;
    float volume;
    bool is3D;
};

// ============================================
// Mod System
// ============================================
class ModSystem {
private:
    std::string modsPath;
    std::vector<ModDef> mods;
    std::unordered_map<std::string, size_t> modIndexByName;
    ScriptEngine* scriptEngine;

    // Engine system pointers
    World* world;
    BlockRegistry* blockRegistry;
    EntityManager* entityManager;
    CraftingSystem* craftingSystem;
    AudioSystem* audioSystem;

    // Registration tracking
    std::vector<BlockRegistration> registeredBlocks;
    std::vector<EntityRegistration> registeredEntities;
    std::vector<ItemRegistration> registeredItems;
    std::vector<RecipeRegistration> registeredRecipes;
    std::vector<BiomeRegistration> registeredBiomes;
    std::vector<SoundRegistration> registeredSounds;

    // Dependency resolution
    bool ResolveDependencies();
    bool CheckDependencies(const ModManifest& manifest, std::string& missing);
    std::vector<size_t> GetLoadOrder();

    // Loading
    bool LoadMod(ModDef& mod);
    bool ExecuteModEntryPoint(ModDef& mod);

public:
    ModSystem();
    ~ModSystem();

    // Lifecycle
    bool Initialize(const std::string& modsDirectory, ScriptEngine* engine);
    void Shutdown();
    void DiscoverMods();
    bool LoadAllMods();
    bool LoadMod(const std::string& modName);
    bool UnloadMod(const std::string& modName);
    bool ReloadMod(const std::string& modName);

    // Queries
    bool IsModLoaded(const std::string& modName) const;
    bool IsModEnabled(const std::string& modName) const;
    const ModManifest* GetModManifest(const std::string& modName) const;
    std::vector<std::string> GetLoadedMods() const;
    std::vector<std::string> GetAvailableMods() const;
    std::vector<std::string> GetModDependencies(const std::string& modName) const;

    // Registration API (called from Lua)
    bool RegisterBlock(const BlockRegistration& block);
    bool RegisterEntity(const EntityRegistration& entity);
    bool RegisterItem(const ItemRegistration& item);
    bool RegisterRecipe(const RecipeRegistration& recipe);
    bool RegisterBiome(const BiomeRegistration& biome);
    bool RegisterSound(const SoundRegistration& sound);

    // Get registered content
    const std::vector<BlockRegistration>& GetRegisteredBlocks() const { return registeredBlocks; }
    const std::vector<EntityRegistration>& GetRegisteredEntities() const { return registeredEntities; }
    const std::vector<ItemRegistration>& GetRegisteredItems() const { return registeredItems; }
    const std::vector<RecipeRegistration>& GetRegisteredRecipes() const { return registeredRecipes; }
    const std::vector<BiomeRegistration>& GetRegisteredBiomes() const { return registeredBiomes; }
    const std::vector<SoundRegistration>& GetRegisteredSounds() const { return registeredSounds; }

    // System setters
    void SetWorld(World* w) { world = w; }
    void SetBlockRegistry(BlockRegistry* reg) { blockRegistry = reg; }
    void SetEntityManager(EntityManager* em) { entityManager = em; }
    void SetCraftingSystem(CraftingSystem* craft) { craftingSystem = craft; }
    void SetAudioSystem(AudioSystem* audio) { audioSystem = audio; }

    // Stats
    size_t GetModCount() const { return mods.size(); }
    size_t GetLoadedModCount() const;
};

} // namespace vge
