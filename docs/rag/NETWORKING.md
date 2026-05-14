# Networking System

## Quick Reference

```cpp
// Create server
vge::GameServer server(7777);
server.Start();

// Create client
vge::GameClient client;
client.Connect("localhost", 7777);

// Main loop
while (running) {
    server.Update(deltaTime);  // If hosting
    client.Update(deltaTime);   // If playing
}
```

## Architecture

The engine uses a **client-server** model with **authoritative server**:

- Server is source of truth for all game state
- Clients predict local movement for responsiveness
- Server validates all actions (anti-cheat)
- Clients interpolate other players for smoothness

## Message Types

### Client → Server
- `PlayerInput` - Movement, look, actions (unreliable, 30Hz)
- `BlockBreak` - Request break block (reliable)
- `BlockPlace` - Request place block (reliable)
- `ChatMessage` - Text chat (reliable)

### Server → Client
- `EntityUpdate` - Position/rotation (unreliable, 30Hz)
- `PlayerSpawn` - Initial spawn data (reliable)
- `BlockUpdate` - Block changes (reliable)
- `TimeSync` - Day/night cycle (reliable, 5s interval)
- `ChatBroadcast` - Chat messages (reliable)

## LAN Discovery

```cpp
vge::LANDiscovery discovery;

// Host: Start broadcasting
discovery.StartBroadcasting("My Server", 7777, 20);

// Client: Discover servers
discovery.StartDiscovery();
auto servers = discovery.GetDiscoveredServers();

for (const auto& server : servers) {
    std::cout << server.name << " at " << server.address 
              << " (" << server.playerCount << "/" << server.maxPlayers << ")" << std::endl;
}
```

## Chat System

```cpp
vge::ChatSystem chat;

// Send message
chat.SendMessage("Hello world!", playerId, "PlayerName");

// Receive messages
chat.OnMessageReceived([](const vge::ChatMessage& msg) {
    std::cout << msg.senderName << ": " << msg.message << std::endl;
});

// System messages
chat.SendSystemMessage("Player joined the game");
```

## Player Skins

```cpp
vge::SkinManager skins;

// Load custom skin
skins.LoadSkin("custom", "path/to/skin.png");

// Set player skin
skins.SetPlayerSkin(playerId, "steve");

// Get available skins
auto skinNames = skins.GetSkinNames();
```

## Anti-Cheat

Server validates:
- Block break/place distance ≤ 6 blocks
- Movement speed ≤ sprint max
- All actions within reasonable bounds

## Client Prediction

1. Client predicts movement immediately
2. Sends input to server
3. Server returns confirmed state
4. Client reconciles if different

## Entity Interpolation

Remote players are rendered 100ms behind actual position for smooth movement using linear interpolation.

## Usage Examples

### Host a Game
```cpp
vge::MultiplayerGame game;
game.HostServer(7777, 20); // Port, max players
game.Run();
```

### Join a Game
```cpp
vge::MultiplayerGame game;
game.JoinServer("192.168.1.100", 7777);
game.Run();
```

### Single Player
```cpp
vge::MultiplayerGame game;
game.StartSinglePlayer(); // Local server + client
game.Run();
```

## Files
- `src/network/game_server.h/cpp` - Server implementation
- `src/network/game_client.h/cpp` - Client implementation
- `src/network/multiplayer_game.h/cpp` - Wrapper class
- `src/network/game_network_protocol.h` - Message types
- `src/network/lan_discovery.h/cpp` - LAN discovery
- `src/network/chat_system.h/cpp` - Chat system
- `src/network/player_skin.h/cpp` - Skin management
