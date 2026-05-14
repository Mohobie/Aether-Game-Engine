#include "serializer.h"
#include "core/save_system.h"
#include <fstream>
#include <iterator>

namespace vge {

bool Serializer::saveWorld(const World& world, const std::string& path) {
    return SaveSystem::SaveWorld(world, path);
}

bool Serializer::loadWorld(World& world, const std::string& path) {
    return SaveSystem::LoadWorld(world, path);
}

bool Serializer::saveEntities(const EntityManager& manager, const std::string& path) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    std::vector<uint8_t> data;
    writeInt32(data, 1); // Entity save format version

    const auto entities = manager.GetAllEntities();
    writeInt32(data, static_cast<int32_t>(entities.size()));

    for (const Entity* entity : entities) {
        writeInt32(data, static_cast<int32_t>(entity->GetID()));
        writeString(data, entity->GetName());
        data.push_back(entity->IsActive() ? 1 : 0);
    }

    file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
    return file.good();
}

bool Serializer::loadEntities(EntityManager& manager, const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    const std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)),
                                    std::istreambuf_iterator<char>());
    if (data.size() < sizeof(int32_t) * 2) {
        return false;
    }

    size_t offset = 0;
    const int32_t version = readInt32(data, offset);
    if (version != 1) {
        return false;
    }

    const int32_t entityCount = readInt32(data, offset);
    if (entityCount < 0) {
        return false;
    }

    manager.Clear();

    for (int32_t i = 0; i < entityCount; ++i) {
        if (offset + sizeof(int32_t) > data.size()) {
            return false;
        }

        (void)readInt32(data, offset); // Persisted ID reserved for future remapping.
        const std::string name = readString(data, offset);
        if (offset >= data.size()) {
            return false;
        }

        const bool active = data[offset++] != 0;
        Entity* entity = manager.CreateEntity(name);
        entity->SetActive(active);
    }

    return true;
}

void Serializer::writeInt32(std::vector<uint8_t>& data, int32_t value) {
    data.push_back((value >> 0) & 0xFF);
    data.push_back((value >> 8) & 0xFF);
    data.push_back((value >> 16) & 0xFF);
    data.push_back((value >> 24) & 0xFF);
}

void Serializer::writeString(std::vector<uint8_t>& data, const std::string& value) {
    writeInt32(data, static_cast<int32_t>(value.size()));
    data.insert(data.end(), value.begin(), value.end());
}

int32_t Serializer::readInt32(const std::vector<uint8_t>& data, size_t& offset) {
    if (offset + sizeof(int32_t) > data.size()) {
        offset = data.size();
        return 0;
    }

    int32_t value = 0;
    value |= (data[offset++] << 0);
    value |= (data[offset++] << 8);
    value |= (data[offset++] << 16);
    value |= (data[offset++] << 24);
    return value;
}

std::string Serializer::readString(const std::vector<uint8_t>& data, size_t& offset) {
    int32_t size = readInt32(data, offset);
    if (size < 0 || offset + static_cast<size_t>(size) > data.size()) {
        offset = data.size();
        return {};
    }

    std::string value(data.begin() + offset, data.begin() + offset + size);
    offset += size;
    return value;
}
} // namespace vge
