#include "game_server.h"
#include "game_network_protocol.h"
#include "voxel/world_generator.h"
#include <iostream>
#include <chrono>

namespace vge {

GameServer::GameServer(uint16_t listenPort)
    : port(listenPort), running(false), nextEntityId(1), maxPlayers(20),
      dayTime(6.0f), dayDuration(1200.0f) { // 20 minute days
}

GameServer::~GameServer() {
    Stop();
}

bool GameServer::Start() {
    // Create network server
    networkServer = std::make_unique<NetworkServer2>(port);
    
    if (!networkServer->Start()) {
        std::cerr << "[GameServer] Failed to start network server on port " << port << std::endl;
        return false;
    }

    // Create world
    world = std::make_unique<World>();
    worldSeed = static_cast<int>(std::chrono::system_clock::now().time_since_epoch().count());
    world->SetSeed(worldSeed);
    
    // Generate initial spawn area
    std::cout << "[GameServer] Generating world with seed " << worldSeed << "..." << std::endl;
    WorldGenerator::GenerateHillyWorld(*world, 50);
    std::cout << "[GameServer] World generation complete!" << std::endl;

    // Register network callbacks
    networkServer->OnClientConnected([this](ClientID clientId) {
        OnClientConnected(clientId);
    });
    
    networkServer->OnClientDisconnected([this](ClientID clientId) {
        OnClientDisconnected(clientId);
    });

    running = true;
    std::cout << "[GameServer] Started on port " << port << std::endl;
    std::cout << "[GameServer] Max players: " << maxPlayers << std::endl;
    return true;
}

void GameServer::Stop() {
    running = false;
    
    // Disconnect all players
    for (auto& [clientId, player] : players) {
        networkServer->DisconnectClient(clientId);
    }
    players.clear();
    
    if (networkServer) {
        networkServer->Stop();
        networkServer.reset();
    }
    
    std::cout << "[GameServer] Stopped" << std::endl;
}

bool GameServer::IsRunning() const {
    return running && networkServer && networkServer->IsRunning();
}

void GameServer::Update(float deltaTime) {
    if (!running) return;

    // Update network
    networkServer->Update(deltaTime);
    
    // Process incoming messages
    ProcessNetworkMessages();
    
    // Update players (physics, input processing)
    UpdatePlayers(deltaTime);
    
    // Update world (mob AI, block updates, etc.)
    UpdateWorld(deltaTime);
    
    // Sync entity states to clients
    SyncEntitiesToClients();
    
    // Sync time
    SyncTimeToClients();
}

void GameServer::ProcessNetworkMessages() {
    // Process all queued messages from clients
    // In a real implementation, this would deserialize and handle messages
    // For now, we use the RPC system
}

void GameServer::UpdatePlayers(float deltaTime) {
    for (auto& [clientId, player] : players) {
        // Apply input to velocity
        float speed = player->lastInput.sprint ? 10.0f : 5.0f;
        
        Vec3 forward(
            std::sin(player->yaw * 3.14159f / 180.0f),
            0,
            std::cos(player->yaw * 3.14159f / 180.0f)
        );
        
        Vec3 right(
            std::cos(player->yaw * 3.14159f / 180.0f),
            0,
            -std::sin(player->yaw * 3.14159f / 180.0f)
        );
        
        Vec3 moveDir = forward * player->lastInput.moveForward + 
                       right * player->lastInput.moveRight;
        
        // Normalize if moving
        float len = std::sqrt(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
        if (len > 0.001f) {
            moveDir.x /= len;
            moveDir.z /= len;
        }
        
        player->velocity.x = moveDir.x * speed;
        player->velocity.z = moveDir.z * speed;
        
        // Apply gravity
        player->velocity.y -= 25.0f * deltaTime;
        
        // Jump
        if (player->lastInput.jump && player->onGround) {
            player->velocity.y = 12.0f;
            player->onGround = false;
        }
        
        // Update position with simple collision
        Vec3 newPos = player->position;
        newPos.x += player->velocity.x * deltaTime;
        newPos.z += player->velocity.z * deltaTime;
        newPos.y += player->velocity.y * deltaTime;
        
        // Simple ground collision (check if feet are on ground)
        int groundX = static_cast<int>(std::floor(newPos.x));
        int groundY = static_cast<int>(std::floor(newPos.y - 0.1f));
        int groundZ = static_cast<int>(std::floor(newPos.z));
        
        BlockTypeID groundBlock = world->GetBlock(groundX, groundY, groundZ);
        if (groundBlock != BlockRegistry::GetInstance().GetBlockId("air")) {
            // On ground
            newPos.y = groundY + 1.0f;
            player->velocity.y = 0;
            player->onGround = true;
        } else {
            player->onGround = false;
        }
        
        player->position = newPos;
        player->yaw = player->lastInput.yaw;
        player->pitch = player->lastInput.pitch;
    }
}

void GameServer::UpdateWorld(float deltaTime) {
    // Update day/night cycle
    dayTime += (24.0f / dayDuration) * deltaTime;
    if (dayTime >= 24.0f) {
        dayTime -= 24.0f;
    }
}

void GameServer::SyncEntitiesToClients() {
    // Send entity updates to all clients
    for (auto& [clientId, player] : players) {
        // Send all other players' positions
        for (auto& [otherId, otherPlayer] : players) {
            if (otherId == clientId) continue;
            
            EntityUpdateMsg update;
            update.entityId = otherPlayer->entityId;
            update.position = otherPlayer->position;
            update.yaw = otherPlayer->yaw;
            update.pitch = otherPlayer->pitch;
            update.velocity = otherPlayer->velocity;
            
            NetBuffer buffer;
            buffer.WriteInt(static_cast<int32_t>(GameMessageType::EntityUpdate));
            update.Serialize(buffer);
            
            SendToClient(clientId, buffer.GetData(), false); // Unreliable for position
        }
        
        // Send own position (for reconciliation)
        EntityUpdateMsg selfUpdate;
        selfUpdate.entityId = player->entityId;
        selfUpdate.position = player->position;
        selfUpdate.yaw = player->yaw;
        selfUpdate.pitch = player->pitch;
        selfUpdate.velocity = player->velocity;
        
        NetBuffer selfBuffer;
        selfBuffer.WriteInt(static_cast<int32_t>(GameMessageType::EntityUpdate));
        selfUpdate.Serialize(selfBuffer);
        
        SendToClient(clientId, selfBuffer.GetData(), false);
    }
}

void GameServer::SyncTimeToClients() {
    static float timeSyncTimer = 0;
    timeSyncTimer += 0.016f; // Approximate delta
    
    if (timeSyncTimer >= 5.0f) { // Sync every 5 seconds
        timeSyncTimer = 0;
        
        TimeSyncMsg timeMsg;
        timeMsg.dayTime = dayTime;
        timeMsg.dayDuration = dayDuration;
        timeMsg.serverTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        
        NetBuffer buffer;
        buffer.WriteInt(static_cast<int32_t>(GameMessageType::TimeSync));
        timeMsg.Serialize(buffer);
        
        BroadcastToAll(buffer.GetData(), true);
    }
}

void GameServer::OnClientConnected(ClientID clientId) {
    if (players.size() >= maxPlayers) {
        std::cout << "[GameServer] Server full, rejecting client " << clientId << std::endl;
        networkServer->DisconnectClient(clientId);
        return;
    }
    
    std::cout << "[GameServer] Client " << clientId << " connected" << std::endl;
    
    // Create player
    auto player = std::make_unique<ServerPlayer>();
    player->clientId = clientId;
    player->entityId = GenerateEntityId();
    
    // Find spawn position (on top of terrain)
    player->position = Vec3(0, 20, 0);
    for (int y = 50; y >= 0; y--) {
        if (world->GetBlock(0, y, 0) != BlockRegistry::GetInstance().GetBlockId("air")) {
            player->position.y = y + 2;
            break;
        }
    }
    
    // Send spawn message to client
    PlayerSpawnMsg spawnMsg;
    spawnMsg.entityId = player->entityId;
    spawnMsg.spawnPosition = player->position;
    spawnMsg.spawnYaw = player->yaw;
    spawnMsg.spawnPitch = player->pitch;
    
    NetBuffer buffer;
    buffer.WriteInt(static_cast<int32_t>(GameMessageType::PlayerSpawn));
    spawnMsg.Serialize(buffer);
    
    SendToClient(clientId, buffer.GetData(), true);
    
    // Store player
    players[clientId] = std::move(player);
    
    // Notify other players
    ChatMsg joinMsg;
    joinMsg.senderId = 0; // Server
    joinMsg.message = "Player " + std::to_string(clientId) + " joined the game";
    
    NetBuffer joinBuffer;
    joinBuffer.WriteInt(static_cast<int32_t>(GameMessageType::ChatBroadcast));
    joinMsg.Serialize(joinBuffer);
    
    BroadcastToAll(joinBuffer.GetData(), true);
}

void GameServer::OnClientDisconnected(ClientID clientId) {
    std::cout << "[GameServer] Client " << clientId << " disconnected" << std::endl;
    
    // Remove player
    players.erase(clientId);
    
    // Notify other players
    ChatMsg leaveMsg;
    leaveMsg.senderId = 0;
    leaveMsg.message = "Player " + std::to_string(clientId) + " left the game";
    
    NetBuffer buffer;
    buffer.WriteInt(static_cast<int32_t>(GameMessageType::ChatBroadcast));
    leaveMsg.Serialize(buffer);
    
    BroadcastToAll(buffer.GetData(), true);
}

void GameServer::HandlePlayerInput(ClientID clientId, const PlayerInputMsg& input) {
    ServerPlayer* player = GetPlayer(clientId);
    if (!player) return;
    
    player->lastInput = input;
}

void GameServer::HandleBlockBreak(ClientID clientId, const BlockBreakMsg& msg) {
    ServerPlayer* player = GetPlayer(clientId);
    if (!player) return;
    
    // Validate distance (anti-cheat)
    Vec3 blockPos(msg.x + 0.5f, msg.y + 0.5f, msg.z + 0.5f);
    float dist = (player->position - blockPos).length();
    if (dist > 6.0f) {
        std::cout << "[GameServer] Player " << clientId << " tried to break block too far away" << std::endl;
        return;
    }
    
    // Break block
    world->SetBlock(msg.x, msg.y, msg.z, BlockRegistry::GetInstance().GetBlockId("air"));
    
    // Broadcast to all clients
    BlockUpdateMsg update;
    update.x = msg.x;
    update.y = msg.y;
    update.z = msg.z;
    update.blockType = BlockRegistry::GetInstance().GetBlockId("air");
    
    NetBuffer buffer;
    buffer.WriteInt(static_cast<int32_t>(GameMessageType::BlockUpdate));
    update.Serialize(buffer);
    
    BroadcastToAll(buffer.GetData(), true);
}

void GameServer::HandleBlockPlace(ClientID clientId, const BlockPlaceMsg& msg) {
    ServerPlayer* player = GetPlayer(clientId);
    if (!player) return;
    
    // Validate distance
    Vec3 blockPos(msg.x + 0.5f, msg.y + 0.5f, msg.z + 0.5f);
    float dist = (player->position - blockPos).length();
    if (dist > 6.0f) {
        std::cout << "[GameServer] Player " << clientId << " tried to place block too far away" << std::endl;
        return;
    }
    
    // Place block
    world->SetBlock(msg.x, msg.y, msg.z, msg.blockType);
    
    // Broadcast to all clients
    BlockUpdateMsg update;
    update.x = msg.x;
    update.y = msg.y;
    update.z = msg.z;
    update.blockType = msg.blockType;
    
    NetBuffer buffer;
    buffer.WriteInt(static_cast<int32_t>(GameMessageType::BlockUpdate));
    update.Serialize(buffer);
    
    BroadcastToAll(buffer.GetData(), true);
}

void GameServer::HandleChatMessage(ClientID clientId, const ChatMsg& msg) {
    // Broadcast to all clients
    NetBuffer buffer;
    buffer.WriteInt(static_cast<int32_t>(GameMessageType::ChatBroadcast));
    
    ChatMsg broadcast;
    broadcast.senderId = clientId;
    broadcast.message = msg.message;
    broadcast.Serialize(buffer);
    
    BroadcastToAll(buffer.GetData(), true);
}

void GameServer::KickPlayer(ClientID clientId, const std::string& reason) {
    std::cout << "[GameServer] Kicking player " << clientId << ": " << reason << std::endl;
    networkServer->DisconnectClient(clientId);
}

ServerPlayer* GameServer::GetPlayer(ClientID clientId) {
    auto it = players.find(clientId);
    if (it != players.end()) {
        return it->second.get();
    }
    return nullptr;
}

void GameServer::BroadcastToAll(const std::vector<uint8_t>& data, bool reliable) {
    for (auto& [clientId, player] : players) {
        SendToClient(clientId, data, reliable);
    }
}

void GameServer::SendToClient(ClientID clientId, const std::vector<uint8_t>& data, bool reliable) {
    if (reliable) {
        networkServer->SendReliable(clientId, data);
    } else {
        networkServer->SendUnreliable(clientId, data);
    }
}

} // namespace vge
