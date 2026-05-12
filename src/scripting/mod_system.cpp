#include "scripting/mod_system.h"
#include "scripting/script_engine.h"
#include "scripting/lua_engine.h"
#include "voxel/world.h"
#include "voxel/block_registry.h"
#include "entity/entity.h"
#include "core/crafting.h"
#include "audio/audio_system.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

namespace vge {

// ============================================
// ModManifest Implementation
// ============================================

bool ModManifest::LoadFromJson(const std::string& json) {
    // Simple JSON parsing (no external dependency)
    // Extract fields using string operations

    auto extractString = [&json](const std::string& key, std::string& out) -> bool {
        size_t pos = json.find("\"" + key + "\"");
        if (pos == std::string::npos) return false;
        pos = json.find(':', pos);
        if (pos == std::string::npos) return false;
        pos = json.find('"', pos);
        if (pos == std::string::npos) return false;
        size_t end = json.find('"', pos + 1);
        if (end == std::string::npos) return false;
        out = json.substr(pos + 1, end - pos - 1);
        return true;
    };

    auto extractStringArray = [&json](const std::string& key, std::vector<std::string>& out) -> bool {
        size_t pos = json.find("\"" + key + "\"");
        if (pos == std::string::npos) return false;
        pos = json.find('[', pos);
        if (pos == std::string::npos) return false;
        size_t end = json.find(']', pos);
        if (end == std::string::npos) return false;

        std::string arrayContent = json.substr(pos + 1, end - pos - 1);
        size_t start = 0;
        while (start < arrayContent.size()) {
            size_t quoteStart = arrayContent.find('"', start);
            if (quoteStart == std::string::npos) break;
            size_t quoteEnd = arrayContent.find('"', quoteStart + 1);
            if (quoteEnd == std::string::npos) break;
            out.push_back(arrayContent.substr(quoteStart + 1, quoteEnd - quoteStart - 1));
            start = quoteEnd + 1;
        }
        return true;
    };

    if (!extractString("name", name)) return false;
    extractString("version", version);
    extractString("author", author);
    extractString("description", description);
    extractString("entry_point", entryPoint);
    extractStringArray("dependencies", dependencies);
    extractStringArray("optionalDependencies", optionalDependencies);

    // Default entry point
    if (entryPoint.empty()) {
        entryPoint = "main.lua";
    }

    return !name.empty();
}

bool ModManifest::LoadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return LoadFromJson(buffer.str());
}

std::string ModManifest::ToJson() const {
    std::stringstream json;
    json << "{\n";
    json << "  \"name\": \"" << name << "\",\n";
    json << "  \"version\": \"" << version << "\",\n";
    json << "  \"author\": \"" << author << "\",\n";
    json << "  \"description\": \"" << description << "\",\n";
    json << "  \"entry_point\": \"" << entryPoint << "\",\n";

    json << "  \"dependencies\": [";
    for (size_t i = 0; i < dependencies.size(); ++i) {
        if (i > 0) json << ", ";
        json << "\"" << dependencies[i] << "\"";
    }
    json << "],\n";

    json << "  \"optionalDependencies\": [";
    for (size_t i = 0; i < optionalDependencies.size(); ++i) {
        if (i > 0) json << ", ";
        json << "\"" << optionalDependencies[i] << "\"";
    }
    json << "]\n";

    json << "}";
    return json.str();
}

// ============================================
// ModSystem Implementation
// ============================================

ModSystem::ModSystem()
    : scriptEngine(nullptr)
    , world(nullptr)
    , blockRegistry(nullptr)
    , entityManager(nullptr)
    , craftingSystem(nullptr)
    , audioSystem(nullptr)
{}

ModSystem::~ModSystem() {
    Shutdown();
}

bool ModSystem::Initialize(const std::string& modsDirectory, ScriptEngine* engine) {
    modsPath = modsDirectory;
    scriptEngine = engine;

    if (!scriptEngine) {
        std::cerr << "[ModSystem] ScriptEngine is null" << std::endl;
        return false;
    }

    std::cout << "[ModSystem] Initialized with path: " << modsPath << std::endl;
    return true;
}

void ModSystem::Shutdown() {
    for (auto& mod : mods) {
        if (mod.loaded) {
            std::cout << "[ModSystem] Unloading mod: " << mod.manifest.name << std::endl;
            mod.loaded = false;
        }
    }
    mods.clear();
    modIndexByName.clear();
    registeredBlocks.clear();
    registeredEntities.clear();
    registeredItems.clear();
    registeredRecipes.clear();
    registeredBiomes.clear();
    registeredSounds.clear();
    std::cout << "[ModSystem] Shutdown complete" << std::endl;
}

void ModSystem::DiscoverMods() {
    mods.clear();
    modIndexByName.clear();

    // Check if mods directory exists
    if (!std::filesystem::exists(modsPath)) {
        std::cout << "[ModSystem] Mods directory does not exist: " << modsPath << std::endl;
        return;
    }

    // Iterate through subdirectories
    for (const auto& entry : std::filesystem::directory_iterator(modsPath)) {
        if (!entry.is_directory()) continue;

        std::string modDir = entry.path().string();
        std::string manifestPath = modDir + "/mod.json";

        if (!std::filesystem::exists(manifestPath)) {
            std::cout << "[ModSystem] Skipping directory without mod.json: " << modDir << std::endl;
            continue;
        }

        ModDef mod;
        mod.directory = modDir;

        if (!mod.manifest.LoadFromFile(manifestPath)) {
            std::cerr << "[ModSystem] Failed to parse manifest: " << manifestPath << std::endl;
            mod.hasErrors = true;
            mod.errorMessage = "Failed to parse mod.json";
        }

        modIndexByName[mod.manifest.name] = mods.size();
        mods.push_back(std::move(mod));

        std::cout << "[ModSystem] Discovered mod: " << mod.manifest.name
                  << " v" << mod.manifest.version
                  << " by " << mod.manifest.author << std::endl;
    }

    std::cout << "[ModSystem] Discovered " << mods.size() << " mod(s)" << std::endl;
}

bool ModSystem::ResolveDependencies() {
    // Check that all dependencies exist
    for (const auto& mod : mods) {
        for (const auto& dep : mod.manifest.dependencies) {
            if (modIndexByName.find(dep) == modIndexByName.end()) {
                std::cerr << "[ModSystem] Missing dependency: " << dep
                          << " required by " << mod.manifest.name << std::endl;
                return false;
            }
        }
    }
    return true;
}

bool ModSystem::CheckDependencies(const ModManifest& manifest, std::string& missing) {
    for (const auto& dep : manifest.dependencies) {
        if (modIndexByName.find(dep) == modIndexByName.end()) {
            missing = dep;
            return false;
        }
    }
    return true;
}

std::vector<size_t> ModSystem::GetLoadOrder() {
    std::vector<size_t> order;
    std::vector<bool> visited(mods.size(), false);
    std::vector<bool> inStack(mods.size(), false);

    std::function<void(size_t)> visit = [&](size_t idx) {
        if (inStack[idx]) {
            std::cerr << "[ModSystem] Circular dependency detected involving: "
                      << mods[idx].manifest.name << std::endl;
            return;
        }
        if (visited[idx]) return;

        inStack[idx] = true;

        // Visit dependencies first
        for (const auto& depName : mods[idx].manifest.dependencies) {
            auto it = modIndexByName.find(depName);
            if (it != modIndexByName.end()) {
                visit(it->second);
            }
        }

        inStack[idx] = false;
        visited[idx] = true;
        order.push_back(idx);
    };

    for (size_t i = 0; i < mods.size(); ++i) {
        if (!visited[i]) {
            visit(i);
        }
    }

    return order;
}

bool ModSystem::LoadMod(ModDef& mod) {
    if (mod.loaded) return true;
    if (mod.hasErrors) return false;

    std::string missing;
    if (!CheckDependencies(mod.manifest, missing)) {
        std::cerr << "[ModSystem] Cannot load " << mod.manifest.name
                  << ": missing dependency " << missing << std::endl;
        mod.hasErrors = true;
        mod.errorMessage = "Missing dependency: " + missing;
        return false;
    }

    std::string entryPath = mod.directory + "/" + mod.manifest.entryPoint;
    if (!std::filesystem::exists(entryPath)) {
        std::cerr << "[ModSystem] Entry point not found: " << entryPath << std::endl;
        mod.hasErrors = true;
        mod.errorMessage = "Entry point not found: " + mod.manifest.entryPoint;
        return false;
    }

    // Execute the mod's entry point
    if (!ExecuteModEntryPoint(mod)) {
        return false;
    }

    mod.loaded = true;
    std::cout << "[ModSystem] Loaded mod: " << mod.manifest.name << std::endl;
    return true;
}

bool ModSystem::ExecuteModEntryPoint(ModDef& mod) {
    if (!scriptEngine) return false;

    std::string entryPath = mod.directory + "/" + mod.manifest.entryPoint;

    // Read the entry point file
    std::ifstream file(entryPath);
    if (!file.is_open()) {
        mod.hasErrors = true;
        mod.errorMessage = "Cannot open entry point: " + entryPath;
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string code = buffer.str();
    file.close();

    // Set the mod directory for sandboxed file access
    if (scriptEngine->GetLuaEngine()) {
        scriptEngine->GetLuaEngine()->SetGlobal("MOD_PATH", mod.directory);
    }

    // Execute the mod code
    if (!scriptEngine->ExecuteCode(code)) {
        mod.hasErrors = true;
        mod.errorMessage = "Execution error: " + scriptEngine->GetLastError();
        std::cerr << "[ModSystem] Error in " << mod.manifest.name << ": "
                  << mod.errorMessage << std::endl;
        return false;
    }

    return true;
}

bool ModSystem::LoadAllMods() {
    if (!ResolveDependencies()) {
        std::cerr << "[ModSystem] Dependency resolution failed" << std::endl;
        return false;
    }

    std::vector<size_t> loadOrder = GetLoadOrder();
    bool allSuccess = true;

    for (size_t idx : loadOrder) {
        if (!LoadMod(mods[idx])) {
            allSuccess = false;
        }
    }

    std::cout << "[ModSystem] Loaded " << GetLoadedModCount() << "/" << mods.size()
              << " mod(s)" << std::endl;
    return allSuccess;
}

bool ModSystem::LoadMod(const std::string& modName) {
    auto it = modIndexByName.find(modName);
    if (it == modIndexByName.end()) {
        std::cerr << "[ModSystem] Mod not found: " << modName << std::endl;
        return false;
    }
    return LoadMod(mods[it->second]);
}

bool ModSystem::UnloadMod(const std::string& modName) {
    auto it = modIndexByName.find(modName);
    if (it == modIndexByName.end()) return false;

    ModDef& mod = mods[it->second];
    if (mod.loaded) {
        mod.loaded = false;
        std::cout << "[ModSystem] Unloaded mod: " << modName << std::endl;
    }
    return true;
}

bool ModSystem::ReloadMod(const std::string& modName) {
    if (!UnloadMod(modName)) return false;
    return LoadMod(modName);
}

bool ModSystem::IsModLoaded(const std::string& modName) const {
    auto it = modIndexByName.find(modName);
    if (it == modIndexByName.end()) return false;
    return mods[it->second].loaded;
}

bool ModSystem::IsModEnabled(const std::string& modName) const {
    auto it = modIndexByName.find(modName);
    if (it == modIndexByName.end()) return false;
    return mods[it->second].manifest.enabled;
}

const ModManifest* ModSystem::GetModManifest(const std::string& modName) const {
    auto it = modIndexByName.find(modName);
    if (it == modIndexByName.end()) return nullptr;
    return &mods[it->second].manifest;
}

std::vector<std::string> ModSystem::GetLoadedMods() const {
    std::vector<std::string> result;
    for (const auto& mod : mods) {
        if (mod.loaded) {
            result.push_back(mod.manifest.name);
        }
    }
    return result;
}

std::vector<std::string> ModSystem::GetAvailableMods() const {
    std::vector<std::string> result;
    for (const auto& mod : mods) {
        result.push_back(mod.manifest.name);
    }
    return result;
}

std::vector<std::string> ModSystem::GetModDependencies(const std::string& modName) const {
    auto it = modIndexByName.find(modName);
    if (it == modIndexByName.end()) return {};
    return mods[it->second].manifest.dependencies;
}

bool ModSystem::RegisterBlock(const BlockRegistration& block) {
    if (!blockRegistry) {
        std::cerr << "[ModSystem] BlockRegistry not set" << std::endl;
        return false;
    }

    BlockDef def;
    def.id = block.id;
    def.name = block.name;
    def.solid = block.solid;
    def.opaque = block.opaque;
    def.hardness = block.hardness;
    def.emission = block.emission;
    def.texture = block.texture;

    // Parse color string "r,g,b"
    if (!block.color.empty()) {
        sscanf(block.color.c_str(), "%f,%f,%f", &def.color.x, &def.color.y, &def.color.z);
    }

    blockRegistry->RegisterBlock(def);
    registeredBlocks.push_back(block);

    std::cout << "[ModSystem] Registered block: " << block.id << std::endl;
    return true;
}

bool ModSystem::RegisterEntity(const EntityRegistration& entity) {
    registeredEntities.push_back(entity);
    std::cout << "[ModSystem] Registered entity: " << entity.id << std::endl;
    return true;
}

bool ModSystem::RegisterItem(const ItemRegistration& item) {
    registeredItems.push_back(item);
    std::cout << "[ModSystem] Registered item: " << item.id << std::endl;
    return true;
}

bool ModSystem::RegisterRecipe(const RecipeRegistration& recipe) {
    if (!craftingSystem) {
        std::cerr << "[ModSystem] CraftingSystem not set" << std::endl;
        return false;
    }

    CraftingRecipe craftRecipe;
    craftRecipe.recipeID = recipe.recipeID;
    craftRecipe.outputItemID = recipe.outputItemID;
    craftRecipe.outputCount = recipe.outputCount;
    craftRecipe.shapeless = recipe.shapeless;

    for (const auto& ing : recipe.ingredients) {
        RecipeIngredient ri;
        ri.itemID = ing.first;
        ri.count = ing.second;
        craftRecipe.ingredients.push_back(ri);
    }

    craftingSystem->AddRecipe(craftRecipe);
    registeredRecipes.push_back(recipe);

    std::cout << "[ModSystem] Registered recipe: " << recipe.recipeID << std::endl;
    return true;
}

bool ModSystem::RegisterBiome(const BiomeRegistration& biome) {
    registeredBiomes.push_back(biome);
    std::cout << "[ModSystem] Registered biome: " << biome.id << std::endl;
    return true;
}

bool ModSystem::RegisterSound(const SoundRegistration& sound) {
    if (!audioSystem) {
        std::cerr << "[ModSystem] AudioSystem not set" << std::endl;
        return false;
    }

    // Load the sound clip
    audioSystem->LoadClip(sound.id, sound.path);
    registeredSounds.push_back(sound);

    std::cout << "[ModSystem] Registered sound: " << sound.id << std::endl;
    return true;
}

size_t ModSystem::GetLoadedModCount() const {
    size_t count = 0;
    for (const auto& mod : mods) {
        if (mod.loaded) ++count;
    }
    return count;
}

} // namespace vge
