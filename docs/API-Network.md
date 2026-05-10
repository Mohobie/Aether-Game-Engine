# Network Module API
**Files:** src/network/client.h, src/network/network_manager.h, src/network/packet.h

## `network/client.h`
```cpp
namespace vge {
```

### `class NetworkClient`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `Connect` | `void` | `const std::string& address, int port` |
| `Disconnect` | `void` | `` |
| `SendMessage` | `void` | `const std::string& msg` |
| `IsConnected` | `bool` | `` |

## `network/network_manager.h`
```cpp
namespace vge {
```

### `class NetworkManager`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `Initialize` | `void` | `` |
| `Connect` | `void` | `const std::string& address, int port` |
| `Disconnect` | `void` | `` |
| `Send` | `void` | `const std::string& message` |
| `Shutdown` | `void` | `` |

## `network/packet.h`
```cpp
namespace vge {
```

### `enum class PacketType : uint8_t`
| Value | Description |
|-------|-------------|
| `Connect` | |
| `Disconnect` | |
| `PlayerPosition` | |
| `BlockChange` | |
| `Chat` | |
| `ChunkData` | |
| `PlayerJoin` | |
| `PlayerLeave` | |

### `class PacketType`

### `class Packet`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `WriteByte` | `void` | `uint8_t value` |
| `WriteInt` | `void` | `int32_t value` |
| `WriteFloat` | `void` | `float value` |
| `WriteString` | `void` | `const std::string& value` |
| `ReadByte` | `uint8_t` | `` |
| `ReadInt` | `int32_t` | `` |
| `ReadFloat` | `float` | `` |
| `ResetRead` | `void` | `` |
| `Clear` | `void` | `` |
