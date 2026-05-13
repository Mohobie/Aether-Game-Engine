#pragma once
#include "network_manager.h"
#include "game_network_protocol.h"
#include "voxel/world.h"
#include "core/player_controller.h"
#include <unordered_map>
#include <memory>
#include <vector>

namespace vge {

// Forward declarations
class NetworkServer2;

// ============================================
// Server Player State
// ============================================
struct ServerPlayer {
    ClientID clientId;
    uint32_t entityId;
    Vec3 position;
    Vec3 velocity;
    float yaw;
    float pitch;
    bool onGround;
    float health;
    float hunger;
    
    // Input state
    PlayerInputMsg lastInput;
    
    ServerPlayer() : clientId(INVALID_CLIENT_ID), entityId(0), 
                     position(0, 50, 0), velocity(0, 0, 0),
                     yaw(0), pitch(0), onGround(false),
                     health(20.0f), hunger(20.0f) {}
};

// ============================================
// Game Server
// ============================================
class GameServer {
public:
    GameServer(uint16_t port);
    ~GameServer();

    // Lifecycle
    bool Start();
    void Stop();
    bool IsRunning() const;

    // Main update loop
    void Update(float deltaTime);

    // World access
    World* GetWorld() { return world.get(); }
    
    // Player management
    size_t GetPlayerCount() const { return players.size(); }
    void KickPlayer(ClientID clientId, const std::string& reason);

    // Configuration
    void SetMaxPlayers(uint32_t max) { maxPlayers = max; }
    uint32_t GetMaxPlayers() const { return maxPlayers; }

private:
    // Network
    std::unique_ptr<NetworkServer2> networkServer;
    uint16_t port;
    bool running;

    // World
    std::unique_ptr<World> world;
    int worldSeed;

    // Players
    std::unordered_map<ClientID, std::unique_ptr<ServerPlayer>> players;
    uint32_t nextEntityId;
    uint32_t maxPlayers;

    // Time
    float dayTime;
    float dayDuration;

    // Update methods
    void ProcessNetworkMessages();
    void UpdatePlayers(float deltaTime);
    void UpdateWorld(float deltaTime);
    void SyncEntitiesToClients();
    void SyncTimeToClients();

    // Client handlers
    void OnClientConnected(ClientID clientId);
    void OnClientDisconnected(ClientID clientId);

    // Message handlers
    void HandlePlayerInput(ClientID clientId, const PlayerInputMsg& input);
    void HandleBlockBreak(ClientID clientId, const BlockBreakMsg& msg);
    void HandleBlockPlace(ClientID clientId, const BlockPlaceMsg& msg);
    void HandleChatMessage(ClientID clientId, const ChatMsg& msg);

    // Helpers
    uint32_t GenerateEntityId() { return nextEntityId++; }
    ServerPlayer* GetPlayer(ClientID clientId);
    void BroadcastToAll(const std::vector<uint8_t>& data, bool reliable = true);
    void SendToClient(ClientID clientId, const std::vector<uint8_t>& data, bool reliable = true);
};

} // namespace vge
