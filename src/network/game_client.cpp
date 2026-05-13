#include "game_client.h"
#include "game_network_protocol.h"
#include <iostream>
#include <cmath>

namespace vge {

GameClient::GameClient()
    : connected(false), localEntityId(0), dayTime(6.0f), dayDuration(1200.0f) {
    
    // Create local player
    localPlayer = std::make_unique<PlayerController>();
    localPlayer->SetPosition(Vec3(0, 50, 0));
    
    // Create camera
    camera = std::make_unique<Camera>();
    camera->SetPosition(Vec3(0, 51.8f, 0));
    camera->SetRotation(0, 0, 0);
    
    // Create world
    world = std::make_unique<World>();
}

GameClient::~GameClient() {
    Disconnect();
}

bool GameClient::Connect(const std::string& address, uint16_t port) {
    networkClient = std::make_unique<NetworkClient2>();
    
    if (!networkClient->Connect(address, port)) {
        std::cerr << "[GameClient] Failed to connect to " << address << ":" << port << std::endl;
        return false;
    }

    connected = true;
    std::cout << "[GameClient] Connected to server" << std::endl;
    
    // Register callbacks
    networkClient->OnConnected([this]() {
        std::cout << "[GameClient] Connection established" << std::endl;
    });
    
    networkClient->OnDisconnected([this]() {
        std::cout << "[GameClient] Disconnected from server" << std::endl;
        connected = false;
    });
    
    return true;
}

void GameClient::Disconnect() {
    if (networkClient) {
        networkClient->Disconnect();
        networkClient.reset();
    }
    connected = false;
    remotePlayers.clear();
}

bool GameClient::IsConnected() const {
    return connected && networkClient && networkClient->IsConnected();
}

void GameClient::Update(float deltaTime) {
    if (!connected) return;

    // Update network
    networkClient->Update(deltaTime);
    
    // Process incoming messages
    ProcessNetworkMessages();
    
    // Interpolate remote players
    InterpolateRemotePlayers(deltaTime);
    
    // Update camera to follow local player
    if (localPlayer) {
        camera->SetPosition(localPlayer->GetPosition() + Vec3(0, 1.6f, 0));
        camera->SetRotation(localPlayer->GetYaw(), localPlayer->GetPitch(), 0);
    }
}

void GameClient::SendPlayerInput(const PlayerInputMsg& input) {
    if (!connected) return;
    
    NetBuffer buffer;
    buffer.WriteInt(static_cast<int32_t>(GameMessageType::PlayerInput));
    input.Serialize(buffer);
    
    networkClient->SendUnreliable(buffer.GetData());
    
    // Predict local movement immediately
    if (localPlayer) {
        // Update local player rotation immediately for responsiveness
        // Position will be updated by server reconciliation
    }
}

void GameClient::SendBlockBreak(int x, int y, int z) {
    if (!connected) return;
    
    BlockBreakMsg msg;
    msg.x = x;
    msg.y = y;
    msg.z = z;
    
    NetBuffer buffer;
    buffer.WriteInt(static_cast<int32_t>(GameMessageType::BlockBreak));
    msg.Serialize(buffer);
    
    networkClient->SendReliable(buffer.GetData());
    
    // Predict block break locally
    world->SetBlock(x, y, z, BlockRegistry::GetInstance().GetBlockId("air"));
}

void GameClient::SendBlockPlace(int x, int y, int z, uint16_t blockType) {
    if (!connected) return;
    
    BlockPlaceMsg msg;
    msg.x = x;
    msg.y = y;
    msg.z = z;
    msg.blockType = blockType;
    
    NetBuffer buffer;
    buffer.WriteInt(static_cast<int32_t>(GameMessageType::BlockPlace));
    msg.Serialize(buffer);
    
    networkClient->SendReliable(buffer.GetData());
    
    // Predict block place locally
    world->SetBlock(x, y, z, blockType);
}

void GameClient::SendChatMessage(const std::string& message) {
    if (!connected) return;
    
    ChatMsg msg;
    msg.senderId = localEntityId;
    msg.message = message;
    
    NetBuffer buffer;
    buffer.WriteInt(static_cast<int32_t>(GameMessageType::ChatMessage));
    msg.Serialize(buffer);
    
    networkClient->SendReliable(buffer.GetData());
}

void GameClient::ProcessNetworkMessages() {
    // In a real implementation, this would process queued messages
    // For now, we use the RPC system
}

void GameClient::InterpolateRemotePlayers(float deltaTime) {
    // Interpolate remote player positions for smooth movement
    for (auto& [entityId, player] : remotePlayers) {
        // Simple linear interpolation
        float t = 10.0f * deltaTime; // Interpolation speed
        if (t > 1.0f) t = 1.0f;
        
        player->position = player->position + (player->targetPosition - player->position) * t;
        
        // Interpolate rotation (handle wrap-around)
        float yawDiff = player->targetYaw - player->yaw;
        while (yawDiff > 180.0f) yawDiff -= 360.0f;
        while (yawDiff < -180.0f) yawDiff += 360.0f;
        player->yaw += yawDiff * t;
        
        float pitchDiff = player->targetPitch - player->pitch;
        while (pitchDiff > 180.0f) pitchDiff -= 360.0f;
        while (pitchDiff < -180.0f) pitchDiff += 360.0f;
        player->pitch += pitchDiff * t;
    }
}

void GameClient::HandlePlayerSpawn(const PlayerSpawnMsg& msg) {
    std::cout << "[GameClient] Spawned with entity ID " << msg.entityId << std::endl;
    
    localEntityId = msg.entityId;
    localPlayer->SetPosition(msg.spawnPosition);
    localPlayer->SetPosition(msg.spawnPosition); // Set initial position
    
    if (onPlayerSpawned) {
        onPlayerSpawned();
    }
}

void GameClient::HandleEntityUpdate(const EntityUpdateMsg& msg) {
    if (msg.entityId == localEntityId) {
        // Server reconciliation for local player
        // In a full implementation, we would compare predicted vs actual position
        // For now, just update position
        localPlayer->SetPosition(msg.position);
        // Don't override yaw/pitch - we want immediate mouse response
    } else {
        // Update remote player
        auto it = remotePlayers.find(msg.entityId);
        if (it != remotePlayers.end()) {
            it->second->targetPosition = msg.position;
            it->second->targetYaw = msg.yaw;
            it->second->targetPitch = msg.pitch;
            it->second->velocity = msg.velocity;
        }
    }
}

void GameClient::HandleEntitySpawn(uint32_t entityId) {
    if (entityId == localEntityId) return;
    
    std::cout << "[GameClient] Remote player spawned: " << entityId << std::endl;
    
    auto player = std::make_unique<RemotePlayer>();
    player->entityId = entityId;
    remotePlayers[entityId] = std::move(player);
}

void GameClient::HandleEntityDestroy(uint32_t entityId) {
    std::cout << "[GameClient] Remote player left: " << entityId << std::endl;
    remotePlayers.erase(entityId);
}

void GameClient::HandleBlockUpdate(const BlockUpdateMsg& msg) {
    world->SetBlock(msg.x, msg.y, msg.z, msg.blockType);
}

void GameClient::HandleChatBroadcast(const ChatMsg& msg) {
    if (onChatMessage) {
        onChatMessage(msg.senderId, msg.message);
    }
}

void GameClient::HandleTimeSync(const TimeSyncMsg& msg) {
    dayTime = msg.dayTime;
    dayDuration = msg.dayDuration;
}

} // namespace vge
