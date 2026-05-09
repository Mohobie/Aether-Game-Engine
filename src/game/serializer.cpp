#include "serializer.h"
#include <fstream>
namespace aether {
bool Serializer::saveWorld(const World& world, const std::string& path) {
    std::ofstream file(path, std::ios::binary);
    return file.good();
}
bool Serializer::loadWorld(World& world, const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    return file.good();
}
bool Serializer::saveEntities(const EntityManager& manager, const std::string& path) {
    std::ofstream file(path, std::ios::binary);
    return file.good();
}
bool Serializer::loadEntities(EntityManager& manager, const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    return file.good();
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
    int32_t value = 0;
    value |= (data[offset++] << 0);
    value |= (data[offset++] << 8);
    value |= (data[offset++] << 16);
    value |= (data[offset++] << 24);
    return value;
}
std::string Serializer::readString(const std::vector<uint8_t>& data, size_t& offset) {
    int32_t size = readInt32(data, offset);
    std::string value(data.begin() + offset, data.begin() + offset + size);
    offset += size;
    return value;
}
} // namespace aether
