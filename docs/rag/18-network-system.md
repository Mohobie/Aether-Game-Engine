# Network System

The engine provides a client-server multiplayer architecture with entity replication, server authority, and efficient chunk streaming.

## Network Architecture

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│   Client    │◄───►│   Server    │◄───►│   Client    │
│  (Player)   │     │  (Authoritative)   │  (Player)   │
└─────────────┘     └─────────────┘     └─────────────┘
       │                   │                   │
       ▼                   ▼                   ▼
  Input commands      World state         Input commands
  (predicted)         (authoritative)     (predicted)
```

## Network Manager

### Configuration

```cpp
vge::NetworkConfig config;
config.role = vge::NetworkRole::Client;  // or Server, Host
config.serverAddress = "192.168.1.100";
config.serverPort = 7777;
config.maxClients = 32;
config.tickRate = 60;  // Network updates per second
config.timeoutSeconds = 30.0f;
config.compressionEnabled = true;

vge::NetworkManager network;
network.Initialize(config);
```

### Client Usage

```cpp
// Connect to server
network.Connect("192.168.1.100", 7777);

// Send input
vge::Packet inputPacket(vge::PacketType::PlayerAction);
inputPacket.WriteVec3(moveDirection);
inputPacket.WriteBool(isJumping);
network.SendPacket(inputPacket);

// Receive world updates
network.SetOnPacketReceived([](const vge::Packet& packet) {
    switch (packet.GetType()) {
        case vge::PacketType::ChunkData:
            // Process chunk data
            break;
        case vge::PacketType::EntityUpdate:
            // Update entity state
            break;
    }
});

// Check connection
if (network.IsConnected()) {
    std::cout << "Ping: " << network.GetPing() << "ms" << std::endl;
}
```

### Server Usage

```cpp
// Start server
config.role = vge::NetworkRole::Server;
network.Initialize(config);
network.StartServer(7777, 32);

// Broadcast to all clients
vge::Packet worldState(vge::PacketType::EntityUpdate);
// ... fill world state
network.SendToAll(worldState);

// Send to specific client
network.SendToClient(clientID, packet);

// Handle client connections
network.SetOnClientConnected([](int clientID) {
    std::cout << "Client " << clientID << " connected" << std::endl;
});

network.SetOnClientDisconnected([](int clientID) {
    std::cout << "Client " << clientID << " disconnected" << std::endl;
});
```

## Entity Replication

### Setting Up Replication

```cpp
vge::EntityReplicator replicator;
replicator.Initialize(&network);

// Register entity for replication
replicator.RegisterEntity(playerEntity, vge::ReplicationMode::Owner);
replicator.RegisterEntity(enemyEntity, vge::ReplicationMode::Server);

// Configure replication
replicator.SetReplicationRate(20);  // 20 updates per second
replicator.SetInterpolation(true);   // Smooth movement
replicator.SetExtrapolation(true);   // Predict movement
```

### Replication Modes

| Mode | Description |
|------|-------------|
| `None` | Not replicated |
| `Owner` | Owner has authority |
| `Server` | Server has authority |
| `All` | Everyone can update |

### Authority

```cpp
// Server sets authority
replicator.SetAuthority(entityID, clientID);

// Check authority
if (replicator.GetAuthority(entityID) == myClientID) {
    // I can modify this entity
}
```

## Server Authority

### Validation

```cpp
vge::ServerAuthority authority;
authority.Initialize(world, &network);

// Validate player actions
authority.SetMaxPlayerSpeed(15.0f);
authority.SetMaxReachDistance(10.0f);

// Process client input
authority.ProcessClientInput(clientID, inputPacket);

// Validate block changes
if (authority.ValidateBlockChange(clientID, x, y, z, blockType)) {
    world.SetBlock(x, y, z, blockType);
}
```

### Cheat Detection

```cpp
authority.SetCheatDetection(true);

// Automatic checks:
// - Speed hacking
// - Reach hacking
// - Invalid block placements
// - Teleportation

// Kick cheaters
authority.KickPlayer(clientID, "Speed hacking detected");
```

## Chunk Streaming

### Server-Side

```cpp
// Send chunks near player
void UpdateChunkStreaming(int clientID, const Vec3& playerPos) {
    int playerChunkX = floor(playerPos.x / CHUNK_SIZE);
    int playerChunkZ = floor(playerPos.z / CHUNK_SIZE);
    
    int viewDistance = 8;
    for (int dx = -viewDistance; dx <= viewDistance; ++dx) {
        for (int dz = -viewDistance; dz <= viewDistance; ++dz) {
            int cx = playerChunkX + dx;
            int cz = playerChunkZ + dz;
            
            if (!client.HasChunk(cx, cz)) {
                Chunk* chunk = world.GetChunk(cx, 0, cz);
                if (chunk) {
                    vge::Packet packet(vge::PacketType::ChunkData);
                    vge::NetworkSerializer::SerializeChunk(*chunk, packet);
                    network.SendToClient(clientID, packet);
                }
            }
        }
    }
}
```

### Client-Side

```cpp
// Receive and load chunks
network.SetOnPacketReceived([](const vge::Packet& packet) {
    if (packet.GetType() == vge::PacketType::ChunkData) {
        vge::Chunk chunk;
        vge::NetworkSerializer::DeserializeChunk(chunk, packet.GetData());
        world.LoadChunk(chunk);
    }
});
```

## Serialization

### Network Serializer

```cpp
// Serialize entity
std::vector<uint8_t> data = vge::NetworkSerializer::SerializeEntity(entity);

// Deserialize entity
vge::NetworkSerializer::DeserializeEntity(entity, data);

// Serialize player state
std::vector<uint8_t> playerData = vge::NetworkSerializer::SerializePlayerState(player);

// Serialize inventory
std::vector<uint8_t> invData = vge::NetworkSerializer::SerializeInventory(inventory);
```

### Compression

```cpp
// Enable compression in config
config.compressionEnabled = true;

// Compress positions
vge::NetworkArchitecture::CompressPosition(position, 0.01f);  // 1cm precision

// Compress rotations
uint16_t compressed = vge::NetworkArchitecture::CompressRotation(angle);
float decompressed = vge::NetworkArchitecture::DecompressRotation(compressed);
```

## Networked Game Mode

```cpp
vge::NetworkedGameMode gameMode;
gameMode.Initialize(world, &network);

// Player management
gameMode.SetMaxPlayers(32);
gameMode.SetSpawnPoint(Vec3(0, 100, 0));

// Spawn player on join
network.SetOnClientConnected([&](int clientID) {
    Entity* player = gameMode.SpawnPlayer(clientID, spawnPoint);
});

// Chat
gameMode.BroadcastChat("Welcome to the server!");
gameMode.SendChatToPlayer(clientID, "Hello!");

// Game mode
 gameMode.SetGameMode("survival");
```

## Best Practices

1. **Server authoritative** - Server validates all actions
2. **Client prediction** - Predict movement locally, correct from server
3. **Entity interpolation** - Smooth between received states
4. **Delta compression** - Only send changed values
5. **Priority system** - Update important entities more frequently
6. **Chunk culling** - Only send visible chunks
7. **Rate limiting** - Limit packet rate per client
8. **Validation** - Validate all client input

## Network Stats

```cpp
vge::NetworkStats stats = vge::NetworkArchitecture::GetNetworkStats(&network);

std::cout << "Bytes sent: " << stats.bytesSent << std::endl;
std::cout << "Bytes received: " << stats.bytesReceived << std::endl;
std::cout << "Packet loss: " << stats.packetLoss << "%" << std::endl;
std::cout << "Latency: " << stats.latency << "ms" << std::endl;
```
