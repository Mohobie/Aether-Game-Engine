#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace vge {

// Packet types for multiplayer
enum class PacketType : uint8_t {
    Connect = 1,
    Disconnect = 2,
    PlayerPosition = 3,
    BlockChange = 4,
    Chat = 5,
    ChunkData = 6,
    PlayerJoin = 7,
    PlayerLeave = 8
};

class Packet {
private:
    std::vector<uint8_t> data;
    int readPos;
    
public:
    Packet();
    
    // Write methods
    void WriteByte(uint8_t value);
    void WriteInt(int32_t value);
    void WriteFloat(float value);
    void WriteString(const std::string& value);
    
    // Read methods
    uint8_t ReadByte();
    int32_t ReadInt();
    float ReadFloat();
    std::string ReadString();
    
    void ResetRead();
    void Clear();
    
    const std::vector<uint8_t>& GetData() const { return data; }
    int GetSize() const { return (int)data.size(); }
    
    // Create specific packet types
    static Packet CreatePlayerPosition(int playerId, float x, float y, float z, float yaw, float pitch);
    static Packet CreateBlockChange(int x, int y, int z, uint8_t blockType);
    static Packet CreateChat(int playerId, const std::string& message);
};

} // namespace vge
