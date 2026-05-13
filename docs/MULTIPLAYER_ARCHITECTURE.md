# Multiplayer Architecture

## Overview

The multiplayer system uses a **client-server** architecture with **authoritative server** design. The server is the source of truth for all game state.

## Network Stack

```
Game Logic (Client/Server)
    ↓
Entity Replicator (syncs entities)
    ↓
Network Manager (RPC + Messages)
    ↓
ENet (UDP-based reliable/unreliable)
    ↓
OS Network Stack
```

## Message Types

### Client → Server
- `PlayerInput` - Movement, look, actions
- `BlockBreak` - Request to break block at position
- `BlockPlace` - Request to place block at position
- `ChatMessage` - Text chat
- `PlayerAction` - Interact, attack, use item

### Server → Client
- `WorldState` - Chunk data, block updates
- `EntityUpdate` - Position/rotation of all entities
- `PlayerSpawn` - Client's player entity created
- `EntitySpawn/Destroy` - Other players/mobs
- `ChatBroadcast` - Chat messages from others
- `PlayerDamage` - Health changes
- `InventoryUpdate` - Item changes

## Authority

| Feature | Authority |
|---------|-----------|
| Player position | Server (with client prediction) |
| Block placement | Server |
| Block breaking | Server |
| Mob AI | Server |
| Health/damage | Server |
| Inventory | Server |
| World generation | Server |
| Day/night cycle | Server |

## Client Prediction

1. Client predicts local player movement immediately
2. Sends input to server
3. Server processes and returns confirmed position
4. Client reconciles if server position differs
5. Other players are interpolated (100ms delay)

## Entity Replication

- **Continuous sync**: Position/rotation at 30Hz (unreliable)
- **On-change sync**: Health, inventory when modified (reliable)
- **Interest management**: Only sync entities within 100 blocks
- **Delta compression**: Only send changed properties

## Implementation Plan

### Phase 1: Basic Connection
- [ ] Server executable
- [ ] Client connection to server
- [ ] Player spawn/sync

### Phase 2: World Sync
- [ ] Chunk streaming to clients
- [ ] Block change replication
- [ ] Initial world download

### Phase 3: Gameplay
- [ ] Player movement with prediction
- [ ] Block break/place
- [ ] Health/damage system

### Phase 4: Polish
- [ ] Latency compensation
- [ ] Lag compensation for combat
- [ ] Reconnection support
