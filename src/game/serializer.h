#pragma once
#include "voxel/world.h"
#include "entity/entity.h"
#include <string>
#include <vector>
namespace vge {
class Serializer {
public:
    static bool saveWorld(const World& world, const std::string& path);
    static bool loadWorld(World& world, const std::string& path);
    static bool saveEntities(const EntityManager& manager, const std::string& path);
    static bool loadEntities(EntityManager& manager, const std::string& path);
private:
    static void writeInt32(std::vector<uint8_t>& data, int32_t value);
    static void writeString(std::vector<uint8_t>& data, const std::string& value);
    static int32_t readInt32(const std::vector<uint8_t>& data, size_t& offset);
    static std::string readString(const std::vector<uint8_t>& data, size_t& offset);
};
} // namespace vge
