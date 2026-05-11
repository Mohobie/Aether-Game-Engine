# Networking System

The engine supports multiplayer through a client-server architecture.

## Network Architecture

```cpp
#include "network/network_architecture.h"

// Server setup
class GameServer {
    vge::NetworkServer server;
    std::vector<PlayerConnection> connections;
    
public:
    void Start(int port) {
        server.Initialize();
        server.Start(port);
        server.SetMaxPlayers(32);
        
        server.OnClientConnected([this](int clientId) {
            OnPlayerConnected(clientId);
        });
        
        server.OnClientDisconnected([this](int clientId) {
            OnPlayerDisconnected(clientId);
        });
        
        server.OnPacketReceived([this](int clientId, Packet& packet) {
            OnPacketReceived(clientId, packet);
        });
    }
    
    void Update(float dt) {
        server.Update(dt);
        
        // Broadcast world state
        BroadcastWorldState();
    }
    
private:
    void OnPlayerConnected(int clientId) {
        // Spawn player entity
        vge::EntityID player = entityManager.CreateEntity();
        
        auto* transform = entityManager.AddComponent<TransformComponent>(player);
        transform->position = GetSpawnPoint();
        
        // Send initial world data
        Packet initPacket;
        initPacket.Write<vge::Vec3>(transform->position);
        initPacket.Write<int>(player);
        server.SendTo(clientId, initPacket, DeliveryMethod::Reliable);
        
        // Notify other players
        Packet joinPacket;
        joinPacket.Write<int>(player);
        server.Broadcast(joinPacket, clientId);
        
        connections.push_back({clientId, player});
    }
    
    void BroadcastWorldState() {
        // Only send changed data
        Packet statePacket;
        statePacket.Write<float>(currentTime);
        
        // Serialize entity transforms
        auto entities = entityManager.Query<TransformComponent>();
        statePacket.Write<int>(entities.size());
        
        for (auto entity : entities) {
            auto* transform = entityManager.GetComponent<TransformComponent>(entity);
            statePacket.Write<vge::EntityID>(entity);
            statePacket.Write<vge::Vec3>(transform->position);
            statePacket.Write<vge::Vec3>(transform->rotation);
        }
        
        server.Broadcast(statePacket, DeliveryMethod::Unreliable);
    }
};

// Client setup
class GameClient {
    vge::NetworkClient client;
    vge::EntityID localPlayer = vge::INVALID_ENTITY;
    
public:
    void Connect(const std::string& ip, int port) {
        client.Initialize();
        client.Connect(ip, port);
        
        client.OnConnected([this]() {
            std::cout << "Connected to server!" << std::endl;
        });
        
        client.OnDisconnected([this]() {
            std::cout << "Disconnected from server" << std::endl;
        });
        
        client.OnPacketReceived([this](Packet& packet) {
            OnPacketReceived(packet);
        });
    }
    
    void Update(float dt) {
        client.Update(dt);
        
        // Send input
        SendInput();
    }
    
    void SendInput() {
        Packet inputPacket;
        inputPacket.Write<float>(input.GetAxis("MoveForward"));
        inputPacket.Write<float>(input.GetAxis("MoveRight"));
        inputPacket.Write<float>(input.GetAxis("LookUp"));
        inputPacket.Write<float>(input.GetAxis("LookRight"));
        inputPacket.Write<bool>(input.GetAction("Jump"));
        inputPacket.Write<bool>(input.GetAction("Attack"));
        
        client.Send(inputPacket, DeliveryMethod::Unreliable);
    }
    
private:
    void OnPacketReceived(Packet& packet) {
        PacketType type = packet.Read<PacketType>();
        
        switch (type) {
            case PacketType::WorldState:
                HandleWorldState(packet);
                break;
            case PacketType::EntitySpawn:
                HandleEntitySpawn(packet);
                break;
            case PacketType::EntityDestroy:
                HandleEntityDestroy(packet);
                break;
            case PacketType::BlockUpdate:
                HandleBlockUpdate(packet);
                break;
        }
    }
    
    void HandleWorldState(Packet& packet) {
        float serverTime = packet.Read<float>();
        int entityCount = packet.Read<int>();
        
        for (int i = 0; i < entityCount; ++i) {
            vge::EntityID entity = packet.Read<vge::EntityID>();
            vge::Vec3 pos = packet.Read<vge::Vec3>();
            vge::Vec3 rot = packet.Read<vge::Vec3>();
            
            // Update entity (with interpolation)
            auto* transform = entityManager.GetComponent<TransformComponent>(entity);
            if (transform) {
                transform->position = vge::Vec3::Lerp(transform->position, pos, 0.3f);
                transform->rotation = rot;
            }
        }
    }
};
```

## Packets

```cpp
#include "network/packet.h"

// Creating packets
Packet packet;
packet.Write<PacketType>(PacketType::PlayerAction);
packet.Write<vge::Vec3>(position);
packet.Write<int>(itemId);
packet.Write<std::string>(message);

// Reading packets
PacketType type = packet.Read<PacketType>();
vge::Vec3 pos = packet.Read<vge::Vec3>();
int id = packet.Read<int>();
std::string msg = packet.Read<std::string>();

// Packet types
enum class PacketType {
    // Connection
    Handshake,
    Disconnect,
    Heartbeat,
    
    // Player
    PlayerInput,
    PlayerState,
    PlayerSpawn,
    PlayerDeath,
    
    // World
    WorldState,
    ChunkData,
    BlockUpdate,
    EntitySpawn,
    EntityDestroy,
    EntityState,
    
    // Gameplay
    ChatMessage,
    InventoryUpdate,
    CraftingRequest,
    DamageEvent,
    SoundEvent,
    
    // System
    RequestChunk,
    TimeSync,
    WeatherUpdate
};

// Delivery methods
enum class DeliveryMethod {
    Reliable,      // TCP-like, guaranteed delivery
    Unreliable,    // UDP-like, may be lost
    Sequenced,     // Unreliable but ordered
    ReliableOrdered // Guaranteed and ordered
};
```

## Entity Synchronization

```cpp
class EntitySync {
    struct EntityState {
        vge::Vec3 position;
        vge::Vec3 rotation;
        vge::Vec3 velocity;
        float timestamp;
    };
    
    std::unordered_map<vge::EntityID, std::deque<EntityState>> stateHistory;
    
public:
    void RecordState(vge::EntityID entity, const EntityState& state) {
        stateHistory[entity].push_back(state);
        
        // Keep only last 1 second of history
        while (stateHistory[entity].size() > 60) {
            stateHistory[entity].pop_front();
        }
    }
    
    void Interpolate(vge::EntityID entity, float renderTime) {
        auto& history = stateHistory[entity];
        if (history.size() < 2) return;
        
        // Find states surrounding renderTime
        for (size_t i = 0; i < history.size() - 1; ++i) {
            if (history[i].timestamp <= renderTime && history[i + 1].timestamp >= renderTime) {
                float t = (renderTime - history[i].timestamp) /
                         (history[i + 1].timestamp - history[i].timestamp);
                
                auto* transform = entityManager.GetComponent<TransformComponent>(entity);
                transform->position = vge::Vec3::Lerp(history[i].position, history[i + 1].position, t);
                transform->rotation = vge::Vec3::Lerp(history[i].rotation, history[i + 1].rotation, t);
                break;
            }
        }
    }
    
    void Reconcile(vge::EntityID entity, const EntityState& serverState, float serverTime) {
        // Client-side prediction reconciliation
        auto* transform = entityManager.GetComponent<TransformComponent>(entity);
        
        // Find state at server time
        auto& history = stateHistory[entity];
        for (auto it = history.begin(); it != history.end(); ++it) {
            if (abs(it->timestamp - serverTime) < 0.001f) {
                // Check if prediction was wrong
                float error = vge::Vec3::Distance(it->position, serverState.position);
                
                if (error > 0.1f) {
                    // Snap to server state
                    transform->position = serverState.position;
                    
                    // Replay inputs since server time
                    ReplayInputs(entity, serverTime);
                }
                
                // Remove old history
                history.erase(history.begin(), it);
                break;
            }
        }
    }
};
```

## Lag Compensation

```cpp
class LagCompensation {
    struct HistoryEntry {
        float timestamp;
        std::unordered_map<vge::EntityID, vge::AABB> hitboxes;
    };
    
    std::deque<HistoryEntry> history;
    
public:
    void RecordState(float time) {
        HistoryEntry entry;
        entry.timestamp = time;
        
        auto entities = entityManager.Query<TransformComponent, PhysicsComponent>();
        for (auto entity : entities) {
            auto* physics = entityManager.GetComponent<PhysicsComponent>(entity);
            entry.hitboxes[entity] = physics->GetHitbox();
        }
        
        history.push_back(entry);
        
        // Keep 1 second of history
        while (!history.empty() && history.front().timestamp < time - 1.0f) {
            history.pop_front();
        }
    }
    
    bool RaycastAtTime(const vge::Ray& ray, float time, vge::RaycastHit& hit) {
        // Find state at target time
        for (const auto& entry : history) {
            if (abs(entry.timestamp - time) < 0.016f) {  // Within one frame
                // Raycast against historical hitboxes
                for (const auto& pair : entry.hitboxes) {
                    if (pair.second.Intersects(ray)) {
                        hit.entity = pair.first;
                        return true;
                    }
                }
                break;
            }
        }
        return false;
    }
};
```

## Chunk Streaming

```cpp
class ChunkStreamer {
    vge::NetworkClient* client;
    std::set<std::pair<int, int>> requestedChunks;
    std::set<std::pair<int, int>> loadedChunks;
    
public:
    void Update(vge::Vec3 playerPos) {
        int playerChunkX = floor(playerPos.x / 16);
        int playerChunkZ = floor(playerPos.z / 16);
        
        // Request chunks within view distance
        int viewDistance = 8;
        for (int dx = -viewDistance; dx <= viewDistance; ++dx) {
            for (int dz = -viewDistance; dz <= viewDistance; ++dz) {
                int cx = playerChunkX + dx;
                int cz = playerChunkZ + dz;
                
                if (loadedChunks.find({cx, cz}) == loadedChunks.end() &&
                    requestedChunks.find({cx, cz}) == requestedChunks.end()) {
                    
                    RequestChunk(cx, cz);
                }
            }
        }
        
        // Unload distant chunks
        for (auto it = loadedChunks.begin(); it != loadedChunks.end();) {
            int dx = it->first - playerChunkX;
            int dz = it->second - playerChunkZ;
            
            if (abs(dx) > viewDistance + 2 || abs(dz) > viewDistance + 2) {
                world.UnloadChunk(it->first, it->second);
                it = loadedChunks.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    void OnChunkReceived(int cx, int cz, const ChunkData& data) {
        world.LoadChunk(cx, cz, data);
        loadedChunks.insert({cx, cz});
        requestedChunks.erase({cx, cz});
    }
    
private:
    void RequestChunk(int cx, int cz) {
        Packet packet;
        packet.Write<PacketType>(PacketType::RequestChunk);
        packet.Write<int>(cx);
        packet.Write<int>(cz);
        
        client->Send(packet, DeliveryMethod::Reliable);
        requestedChunks.insert({cx, cz});
    }
};
```

## Chat System

```cpp
class ChatSystem {
    struct ChatMessage {
        std::string sender;
        std::string text;
        float timestamp;
        vge::Vec4 color;
    };
    
    std::deque<ChatMessage> messages;
    const size_t maxMessages = 100;
    
public:
    void SendMessage(const std::string& text) {
        Packet packet;
        packet.Write<PacketType>(PacketType::ChatMessage);
        packet.Write<std::string>(text);
        
        client->Send(packet, DeliveryMethod::Reliable);
        
        // Add locally for instant feedback
        AddMessage("You", text, vge::Vec4(1, 1, 1, 1));
    }
    
    void ReceiveMessage(const std::string& sender, const std::string& text) {
        vge::Vec4 color = vge::Vec4(1, 1, 1, 1);
        
        // Color based on sender type
        if (sender == "Server") {
            color = vge::Vec4(1, 0.5f, 0, 1);
        } else if (sender == "System") {
            color = vge::Vec4(0, 1, 0, 1);
        }
        
        AddMessage(sender, text, color);
    }
    
    void AddMessage(const std::string& sender, const std::string& text, vge::Vec4 color) {
        ChatMessage msg;
        msg.sender = sender;
        msg.text = text;
        msg.timestamp = GetTime();
        msg.color = color;
        
        messages.push_back(msg);
        
        if (messages.size() > maxMessages) {
            messages.pop_front();
        }
    }
    
    void Render(vge::UISystem* ui) {
        float y = 500;
        for (const auto& msg : messages) {
            if (GetTime() - msg.timestamp > 30.0f && !chatOpen) continue;
            
            std::string display = "[" + msg.sender + "]: " + msg.text;
            ui->DrawText(10, y, display, msg.color);
            y -= 20;
        }
    }
};
```

## Network Debugging

```cpp
class NetworkDebugger {
    bool showDebug = false;
    
    struct NetworkStats {
        float ping;
        int packetsSent;
        int packetsReceived;
        int bytesSent;
        int bytesReceived;
        float packetLoss;
    };
    
    NetworkStats stats;
    
public:
    void Toggle() {
        showDebug = !showDebug;
    }
    
    void Update(float dt) {
        // Update stats from network layer
        stats.ping = network.GetPing();
        stats.packetsSent = network.GetPacketsSent();
        stats.packetsReceived = network.GetPacketsReceived();
        stats.bytesSent = network.GetBytesSent();
        stats.bytesReceived = network.GetBytesReceived();
        stats.packetLoss = network.GetPacketLoss();
    }
    
    void Render(vge::UISystem* ui) {
        if (!showDebug) return;
        
        ui->DrawText(10, 100, "=== Network Stats ===", vge::Vec4(0, 1, 0, 1));
        ui->DrawText(10, 120, "Ping: " + std::to_string((int)stats.ping) + " ms", vge::Vec4(1, 1, 1, 1));
        ui->DrawText(10, 140, "Packets: " + std::to_string(stats.packetsReceived) + "/" + std::to_string(stats.packetsSent), vge::Vec4(1, 1, 1, 1));
        ui->DrawText(10, 160, "Bytes: " + std::to_string(stats.bytesReceived) + "/" + std::to_string(stats.bytesSent), vge::Vec4(1, 1, 1, 1));
        ui->DrawText(10, 180, "Packet Loss: " + std::to_string((int)(stats.packetLoss * 100)) + "%", vge::Vec4(1, 0, 0, 1));
    }
};
```
