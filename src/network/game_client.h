#pragma once
#include "network_manager.h"
#include "game_network_protocol.h"
#include "voxel/world.h"
#include "core/player_controller.h"
#include "rendering/camera.h"
#include <memory>
#include <functional>
#include <queue>

namespace vge {

// ============================================
// Remote Player (other players on server)
// ============================================
struct RemotePlayer {
    uint32_t entityId;
    Vec3 position;
    Vec3 targetPosition; // For interpolation
    Vec3 velocity;
    float yaw;
    float pitch;
    float targetYaw;
    float targetPitch;
    
    RemotePlayer() : entityId(0), position(0, 0, 0), targetPosition(0, 0, 0),
                     velocity(0, 0, 0), yaw(0), pitch(0), targetYaw(0), targetPitch(0) {}
};

// ============================================
// Game Client
// ============================================
class GameClient {
public:
    GameClient();
    ~GameClient();

    // Connection
    bool Connect(const std::string& address, uint16_t port);
    void Disconnect();
    bool IsConnected() const;

    // Main update loop
    void Update(float deltaTime);

    // Input (called from game loop)
    void SendPlayerInput(const PlayerInputMsg& input);
    void SendBlockBreak(int x, int y, int z);
    void SendBlockPlace(int x, int y, int z, uint16_t blockType);
    void SendChatMessage(const std::string& message);

    // State access
    World* GetWorld() { return world.get(); }
    PlayerController* GetLocalPlayer() { return localPlayer.get(); }
    Camera* GetCamera() { return camera.get(); }
    
    // Remote players
    const std::unordered_map<uint32_t, std::unique_ptr<RemotePlayer>>& GetRemotePlayers() const { return remotePlayers; }
    
    // Time
    float GetDayTime() const { return dayTime; }
    
    // Events
    void OnPlayerSpawned(std::function<void()> callback) { onPlayerSpawned = callback; }
    void OnChatMessage(std::function<void(uint32_t, const std::string&)> callback) { onChatMessage = callback; }

private:
    // Network
    std::unique_ptr<NetworkClient2> networkClient;
    bool connected;

    // World
    std::unique_ptr<World> world;
    
    // Local player
    std::unique_ptr<PlayerController> localPlayer;
    std::unique_ptr<Camera> camera;
    uint32_t localEntityId;
    
    // Remote players
    std::unordered_map<uint32_t, std::unique_ptr<RemotePlayer>> remotePlayers;
    
    // Time
    float dayTime;
    float dayDuration;
    
    // Events
    std::function<void()> onPlayerSpawned;
    std::function<void(uint32_t, const std::string&)> onChatMessage;

    // Update methods
    void ProcessNetworkMessages();
    void InterpolateRemotePlayers(float deltaTime);
    void PredictLocalPlayer(float deltaTime);
    void ReconcileWithServer();

    // Message handlers
    void HandlePlayerSpawn(const PlayerSpawnMsg& msg);
    void HandleEntityUpdate(const EntityUpdateMsg& msg);
    void HandleEntitySpawn(uint32_t entityId);
    void HandleEntityDestroy(uint32_t entityId);
    void HandleBlockUpdate(const BlockUpdateMsg& msg);
    void HandleChatBroadcast(const ChatMsg& msg);
    void HandleTimeSync(const TimeSyncMsg& msg);
};

} // namespace vge
