#include "block_types.h"
#include "block_registry.h"
#include "platform/file_system.h"
#include "platform/platform_common.h"
#include <iostream>
#include <fstream>
#include <sstream>

// Simple JSON parser for block definitions
namespace vge {

// Forward declaration for old registry compatibility
extern BlockRegistry& GetOldRegistry();

bool Block::IsSolid() const {
    return BlockRegistry::GetInstance().GetBlock(typeId).IsSolid();
}

bool Block::IsOpaque() const {
    return BlockRegistry::GetInstance().GetBlock(typeId).IsOpaque();
}

const std::string& Block::GetName() const {
    return BlockRegistry::GetInstance().GetBlock(typeId).GetName();
}

Vec3 Block::GetColor() const {
    return BlockRegistry::GetInstance().GetBlock(typeId).GetColor();
}

int Block::GetEmission() const {
    return BlockRegistry::GetInstance().GetBlock(typeId).GetEmission();
}

float Block::GetHardness() const {
    return BlockRegistry::GetInstance().GetBlock(typeId).GetHardness();
}

// ============================================
// BlockRegistry Implementation
// ============================================

BlockRegistry::BlockRegistry() {
    // Register air by default
    BlockDef air;
    air.id = "air";
    air.name = "Air";
    air.solid = false;
    air.opaque = false;
    air.hardness = 0.0f;
    air.color = Vec3(0, 0, 0);
    air.emission = 0;
    blocks.push_back(air);
    idToIndex["air"] = BLOCK_AIR;
}

BlockRegistry& BlockRegistry::GetInstance() {
    static BlockRegistry instance;
    return instance;
}

BlockTypeID BlockRegistry::RegisterBlock(const BlockDef& def) {
    if (idToIndex.find(def.id) != idToIndex.end()) {
        std::cerr << "[BlockRegistry] Block '" << def.id << "' already registered" << std::endl;
        return idToIndex[def.id];
    }
    
    BlockTypeID id = nextId++;
    blocks.push_back(def);
    idToIndex[def.id] = id;
    
    std::cout << "[BlockRegistry] Registered '" << def.id << "' as ID " << id << std::endl;
    return id;
}

const BlockDef& BlockRegistry::GetBlock(BlockTypeID id) const {
    if (id < blocks.size()) {
        return blocks[id];
    }
    // Return air as fallback
    return blocks[BLOCK_AIR];
}

const BlockDef& BlockRegistry::GetBlock(const std::string& id) const {
    auto it = idToIndex.find(id);
    if (it != idToIndex.end()) {
        return blocks[it->second];
    }
    // Return air as fallback
    return blocks[BLOCK_AIR];
}

BlockTypeID BlockRegistry::GetBlockId(const std::string& id) const {
    auto it = idToIndex.find(id);
    if (it != idToIndex.end()) {
        return it->second;
    }
    return BLOCK_AIR;
}

bool BlockRegistry::HasBlock(const std::string& id) const {
    return idToIndex.find(id) != idToIndex.end();
}

bool BlockRegistry::HasBlock(BlockTypeID id) const {
    return id < blocks.size();
}

std::vector<std::string> BlockRegistry::GetAllBlockIds() const {
    std::vector<std::string> result;
    for (const auto& pair : idToIndex) {
        result.push_back(pair.first);
    }
    return result;
}

// Simple JSON parsing (no external dependency)
bool BlockRegistry::LoadFromFile(const std::string& path) {
    std::string json = File::ReadText(path);
    if (json.empty()) {
        std::cerr << "[BlockRegistry] Failed to load: " << path << std::endl;
        return false;
    }
    return LoadFromJson(json);
}

bool BlockRegistry::LoadFromJson(const std::string& json) {
    // Very simple JSON parser - looks for block objects
    size_t pos = 0;
    
    // Find "blocks" array
    size_t blocksPos = json.find("\"blocks\"");
    if (blocksPos == std::string::npos) {
        std::cerr << "[BlockRegistry] No 'blocks' array found" << std::endl;
        return false;
    }
    
    // Find array start
    size_t arrayStart = json.find('[', blocksPos);
    if (arrayStart == std::string::npos) {
        std::cerr << "[BlockRegistry] Invalid blocks format" << std::endl;
        return false;
    }
    
    // Parse each block object
    size_t current = arrayStart + 1;
    int blockCount = 0;
    
    while (current < json.size()) {
        // Find next object start
        size_t objStart = json.find('{', current);
        if (objStart == std::string::npos) break;
        
        size_t objEnd = json.find('}', objStart);
        if (objEnd == std::string::npos) break;
        
        // Parse block object
        std::string obj = json.substr(objStart + 1, objEnd - objStart - 1);
        BlockDef def;
        
        // Extract fields
        auto extractString = [&](const std::string& key) -> std::string {
            size_t keyPos = obj.find("\"" + key + "\"");
            if (keyPos == std::string::npos) return "";
            size_t valStart = obj.find('"', keyPos + key.length() + 2);
            if (valStart == std::string::npos) return "";
            size_t valEnd = obj.find('"', valStart + 1);
            if (valEnd == std::string::npos) return "";
            return obj.substr(valStart + 1, valEnd - valStart - 1);
        };
        
        auto extractBool = [&](const std::string& key) -> bool {
            size_t keyPos = obj.find("\"" + key + "\"");
            if (keyPos == std::string::npos) return false;
            size_t valStart = obj.find(':', keyPos);
            if (valStart == std::string::npos) return false;
            std::string val = obj.substr(valStart + 1, 10);
            return val.find("true") != std::string::npos;
        };
        
        auto extractFloat = [&](const std::string& key) -> float {
            size_t keyPos = obj.find("\"" + key + "\"");
            if (keyPos == std::string::npos) return 0.0f;
            size_t valStart = obj.find(':', keyPos);
            if (valStart == std::string::npos) return 0.0f;
            return std::stof(obj.substr(valStart + 1, 10));
        };
        
        auto extractInt = [&](const std::string& key) -> int {
            size_t keyPos = obj.find("\"" + key + "\"");
            if (keyPos == std::string::npos) return 0;
            size_t valStart = obj.find(':', keyPos);
            if (valStart == std::string::npos) return 0;
            return std::stoi(obj.substr(valStart + 1, 10));
        };
        
        def.id = extractString("id");
        def.name = extractString("name");
        def.solid = extractBool("solid");
        def.opaque = extractBool("opaque");
        def.hardness = extractFloat("hardness");
        def.emission = extractInt("emission");
        
        // Extract color array [r, g, b]
        size_t colorPos = obj.find("\"color\"");
        if (colorPos != std::string::npos) {
            size_t arrStart = obj.find('[', colorPos);
            size_t arrEnd = obj.find(']', arrStart);
            if (arrStart != std::string::npos && arrEnd != std::string::npos) {
                std::string colorStr = obj.substr(arrStart + 1, arrEnd - arrStart - 1);
                // Parse comma-separated floats
                std::stringstream ss(colorStr);
                std::string val;
                int i = 0;
                while (std::getline(ss, val, ',') && i < 3) {
                    float f = std::stof(val);
                    if (i == 0) def.color.x = f;
                    else if (i == 1) def.color.y = f;
                    else if (i == 2) def.color.z = f;
                    i++;
                }
            }
        }
        
        if (!def.id.empty() && def.id != "air") {
            RegisterBlock(def);
            blockCount++;
        }
        
        current = objEnd + 1;
    }
    
    std::cout << "[BlockRegistry] Loaded " << blockCount << " blocks" << std::endl;
    return blockCount > 0;
}

bool BlockRegistry::SaveToFile(const std::string& path) const {
    std::ofstream file(path);
    if (!file.is_open()) return false;
    
    file << "{\n  \"version\": 1,\n  \"blocks\": [\n";
    
    bool first = true;
    for (size_t i = 1; i < blocks.size(); ++i) {  // Skip air
        const auto& def = blocks[i];
        if (!first) file << ",\n";
        first = false;
        
        file << "    {\n";
        file << "      \"id\": \"" << def.id << "\",\n";
        file << "      \"name\": \"" << def.name << "\",\n";
        file << "      \"solid\": " << (def.solid ? "true" : "false") << ",\n";
        file << "      \"opaque\": " << (def.opaque ? "true" : "false") << ",\n";
        file << "      \"hardness\": " << def.hardness << ",\n";
        file << "      \"color\": [" << def.color.x << ", " << def.color.y << ", " << def.color.z << "],\n";
        file << "      \"emission\": " << def.emission << "\n";
        file << "    }";
    }
    
    file << "\n  ]\n}\n";
    return true;
}

} // namespace vge