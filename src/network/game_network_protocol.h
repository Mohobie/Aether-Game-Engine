#pragma once
#include "network_serialization.h"
#include "math/vec3.h"
#include <cstdint>
#include <string>
#include <vector>

namespace vge {

// ============================================
// Message Types
// ============================================
enum class GameMessageType : uint16_t {
    // Client -> Server
    PlayerInput = 1,
    BlockBreak,
    BlockPlace,
    ChatMessage,
    PlayerAction,
    
    // Server -> Client
    WorldState = 128,
    EntityUpdate,
    PlayerSpawn,
    EntitySpawn,
    EntityDestroy,
    ChatBroadcast,
    PlayerDamage,
    InventoryUpdate,
    BlockUpdate,
    ChunkData,
    TimeSync,
    
    // Bidirectional
    Ping = 256,
    Pong
};

// ============================================
// Player Input (Client -> Server)
// ============================================
struct PlayerInputMsg {
    float moveForward;   // -1 to 1
    float moveRight;     // -1 to 1
    float yaw;           // degrees
    float pitch;         // degrees
    bool jump;
    bool sprint;
    bool attack;
    bool use;
    
    void Serialize(NetBuffer& buffer) const {
        buffer.WriteFloat(moveForward);
        buffer.WriteFloat(moveRight);
        buffer.WriteFloat(yaw);
        buffer.WriteFloat(pitch);
        buffer.WriteBool(jump);
        buffer.WriteBool(sprint);
        buffer.WriteBool(attack);
        buffer.WriteBool(use);
    }
    
    void Deserialize(NetBuffer& buffer) {
        moveForward = buffer.ReadFloat();
        moveRight = buffer.ReadFloat();
        yaw = buffer.ReadFloat();
        pitch = buffer.ReadFloat();
        jump = buffer.ReadBool();
        sprint = buffer.ReadBool();
        attack = buffer.ReadBool();
        use = buffer.ReadBool();
    }
};

// ============================================
// Block Break/Place (Client -> Server)
// ============================================
struct BlockBreakMsg {
    int32_t x, y, z;
    
    void Serialize(NetBuffer& buffer) const {
        buffer.WriteInt(x);
        buffer.WriteInt(y);
        buffer.WriteInt(z);
    }
    
    void Deserialize(NetBuffer& buffer) {
        x = buffer.ReadInt();
        y = buffer.ReadInt();
        z = buffer.ReadInt();
    }
};

struct BlockPlaceMsg {
    int32_t x, y, z;
    uint16_t blockType;
    
    void Serialize(NetBuffer& buffer) const {
        buffer.WriteInt(x);
        buffer.WriteInt(y);
        buffer.WriteInt(z);
        buffer.WriteInt(blockType);
    }
    
    void Deserialize(NetBuffer& buffer) {
        x = buffer.ReadInt();
        y = buffer.ReadInt();
        z = buffer.ReadInt();
        blockType = buffer.ReadInt();
    }
};

// ============================================
// Entity Update (Server -> Client)
// ============================================
struct EntityUpdateMsg {
    uint32_t entityId;
    Vec3 position;
    float yaw;
    float pitch;
    Vec3 velocity;
    
    void Serialize(NetBuffer& buffer) const {
        buffer.WriteInt(entityId);
        buffer.WriteVec3(position);
        buffer.WriteFloat(yaw);
        buffer.WriteFloat(pitch);
        buffer.WriteVec3(velocity);
    }
    
    void Deserialize(NetBuffer& buffer) {
        entityId = buffer.ReadInt();
        position = buffer.ReadVec3();
        yaw = buffer.ReadFloat();
        pitch = buffer.ReadFloat();
        velocity = buffer.ReadVec3();
    }
};

// ============================================
// Player Spawn (Server -> Client)
// ============================================
struct PlayerSpawnMsg {
    uint32_t entityId;
    Vec3 spawnPosition;
    float spawnYaw;
    float spawnPitch;
    
    void Serialize(NetBuffer& buffer) const {
        buffer.WriteInt(entityId);
        buffer.WriteVec3(spawnPosition);
        buffer.WriteFloat(spawnYaw);
        buffer.WriteFloat(spawnPitch);
    }
    
    void Deserialize(NetBuffer& buffer) {
        entityId = buffer.ReadInt();
        spawnPosition = buffer.ReadVec3();
        spawnYaw = buffer.ReadFloat();
        spawnPitch = buffer.ReadFloat();
    }
};

// ============================================
// Block Update (Server -> Client)
// ============================================
struct BlockUpdateMsg {
    int32_t x, y, z;
    uint16_t blockType;
    
    void Serialize(NetBuffer& buffer) const {
        buffer.WriteInt(x);
        buffer.WriteInt(y);
        buffer.WriteInt(z);
        buffer.WriteInt(blockType);
    }
    
    void Deserialize(NetBuffer& buffer) {
        x = buffer.ReadInt();
        y = buffer.ReadInt();
        z = buffer.ReadInt();
        blockType = buffer.ReadInt();
    }
};

// ============================================
// Chat Message (Bidirectional)
// ============================================
struct ChatMsg {
    uint32_t senderId;
    std::string message;
    
    void Serialize(NetBuffer& buffer) const {
        buffer.WriteInt(senderId);
        buffer.WriteString(message);
    }
    
    void Deserialize(NetBuffer& buffer) {
        senderId = buffer.ReadInt();
        message = buffer.ReadString();
    }
};

// ============================================
// Time Sync (Server -> Client)
// ============================================
struct TimeSyncMsg {
    float dayTime;       // 0.0 - 24.0
    float dayDuration;   // seconds per day
    uint64_t serverTime; // ms since epoch
    
    void Serialize(NetBuffer& buffer) const {
        buffer.WriteFloat(dayTime);
        buffer.WriteFloat(dayDuration);
        buffer.WriteInt(static_cast<int32_t>(serverTime >> 32));
        buffer.WriteInt(static_cast<int32_t>(serverTime & 0xFFFFFFFF));
    }
    
    void Deserialize(NetBuffer& buffer) {
        dayTime = buffer.ReadFloat();
        dayDuration = buffer.ReadFloat();
        serverTime = (static_cast<uint64_t>(buffer.ReadInt()) << 32) | static_cast<uint32_t>(buffer.ReadInt());
    }
};

// ============================================
// Ping/Pong (Bidirectional - for latency)
// ============================================
struct PingMsg {
    uint64_t timestamp;
    
    void Serialize(NetBuffer& buffer) const {
        buffer.WriteInt(static_cast<int32_t>(timestamp >> 32));
        buffer.WriteInt(static_cast<int32_t>(timestamp & 0xFFFFFFFF));
    }
    
    void Deserialize(NetBuffer& buffer) {
        timestamp = (static_cast<uint64_t>(buffer.ReadInt()) << 32) | static_cast<uint32_t>(buffer.ReadInt());
    }
};

} // namespace vge
