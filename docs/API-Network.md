# Network Module API

**Files:** `network/network_engine.h`, `network/network_client.h`, `network/network_server.h`, `network/packet.h`, `network/connection.h`

---

## `network/network_engine.h`

```cpp
namespace vge {
```

### `enum class NetworkProtocol`
| Value | Description |
|-------|-------------|
| `TCP` | TCP protocol |
| `UDP` | UDP protocol |

### `enum class NetworkRole`
| Value | Description |
|-------|-------------|
| `None` | Not connected |
| `Client` | Client mode |
| `Server` | Server mode |
| `Host` | Host (client + server) |

### `class NetworkEngine`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `GetInstance` | `NetworkEngine&` | `void` | Singleton |
| `Initialize` | `bool` | `void` | Initialize network |
| `Shutdown` | `void` | `void` | Shutdown network |
| `StartServer` | `bool` | `uint16_t port, int maxClients = 32` | Start server |
| `StopServer` | `void` | `void` | Stop server |
| `Connect` | `bool` | `const std::string& address, uint16_t port` | Connect to server |
| `Disconnect` | `void` | `void` | Disconnect |
| `IsConnected` | `bool` | `void` | Check connected |
| `IsServer` | `bool` | `void` | Check is server |
| `GetRole` | `NetworkRole` | `void` | Get network role |
| `Send` | `void` | `const Packet& packet` | Send packet |
| `SendTo` | `void` | `const Packet& packet, ConnectionID id` | Send to specific |
| `SendToAll` | `void` | `const Packet& packet` | Broadcast |
| `SendToAllExcept` | `void` | `const Packet& packet, ConnectionID except` | Broadcast except |
| `Receive` | `bool` | `Packet& packet` | Receive packet |
| `GetConnectionCount` | `size_t` | `void` | Get connection count |
| `GetConnection` | `Connection*` | `ConnectionID id` | Get connection |
| `GetAllConnections` | `std::vector<Connection*>` | `void` | All connections |
| `SetMaxConnections` | `void` | `int max` | Set max connections |
| `GetMaxConnections` | `int` | `void` | Get max connections |
| `SetLatencySimulation` | `void` | `bool enabled, float latency = 0.0f` | Set latency simulation |
| `SetPacketLossSimulation` | `void` | `bool enabled, float lossRate = 0.0f` | Set packet loss |
| `Update` | `void` | `void` | Update network |
| `SetTickRate` | `void` | `int ticksPerSecond` | Set tick rate |
| `GetTickRate` | `int` | `void` | Get tick rate |
| `GetLocalAddress` | `std::string` | `void` | Get local address |
| `GetLocalPort` | `uint16_t` | `void` | Get local port |
| `SetOnConnect` | `void` | `std::function<void(Connection*)> callback` | Set connect callback |
| `SetOnDisconnect` | `void` | `std::function<void(Connection*)> callback` | Set disconnect callback |
| `SetOnReceive` | `void` | `std::function<void(Connection*, Packet&)> callback` | Set receive callback |

---

## `network/network_client.h`

### `class NetworkClient`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `NetworkClient` | (ctor) | `void` | Constructor |
| `Connect` | `bool` | `const std::string& address, uint16_t port` | Connect |
| `Disconnect` | `void` | `void` | Disconnect |
| `IsConnected` | `bool` | `void` | Check connected |
| `Send` | `void` | `const Packet& packet` | Send packet |
| `Receive` | `bool` | `Packet& packet` | Receive packet |
| `GetPing` | `float` | `void` | Get ping (ms) |
| `GetServerAddress` | `std::string` | `void` | Get server address |
| `GetServerPort` | `uint16_t` | `void` | Get server port |
| `SetPlayerName` | `void` | `const std::string& name` | Set player name |
| `GetPlayerName` | `std::string` | `void` | Get player name |
| `SetPlayerID` | `void` | `uint32_t id` | Set player ID |
| `GetPlayerID` | `uint32_t` | `void` | Get player ID |
| `Update` | `void` | `void` | Update client |
| `SetOnConnect` | `void` | `std::function<void()> callback` | Set connect callback |
| `SetOnDisconnect` | `void` | `std::function<void()> callback` | Set disconnect callback |
| `SetOnReceive` | `void` | `std::function<void(Packet&)> callback` | Set receive callback |
| `SetOnPingUpdate` | `void` | `std::function<void(float)> callback` | Set ping callback |

---

## `network/network_server.h`

### `class NetworkServer`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `NetworkServer` | (ctor) | `void` | Constructor |
| `Start` | `bool` | `uint16_t port, int maxClients = 32` | Start server |
| `Stop` | `void` | `void` | Stop server |
| `IsRunning` | `bool` | `void` | Check running |
| `SendTo` | `void` | `const Packet& packet, ConnectionID id` | Send to client |
| `SendToAll` | `void` | `const Packet& packet` | Broadcast |
| `SendToAllExcept` | `void` | `const Packet& packet, ConnectionID except` | Broadcast except |
| `Kick` | `void` | `ConnectionID id, const std::string& reason` | Kick client |
| `Ban` | `void` | `ConnectionID id, const std::string& reason` | Ban client |
| `Unban` | `void` | `const std::string& address` | Unban address |
| `IsBanned` | `bool` | `const std::string& address` | Check banned |
| `GetClientCount` | `size_t` | `void` | Get client count |
| `GetClient` | `Connection*` | `ConnectionID id` | Get client |
| `GetAllClients` | `std::vector<Connection*>` | `void` | All clients |
| `SetMaxClients` | `void` | `int max` | Set max clients |
| `GetMaxClients` | `int` | `void` | Get max clients |
| `SetPassword` | `void` | `const std::string& password` | Set password |
| `HasPassword` | `bool` | `void` | Check has password |
| `Update` | `void` | `void` | Update server |
| `SetTickRate` | `void` | `int ticksPerSecond` | Set tick rate |
| `GetTickRate` | `int` | `void` | Get tick rate |
| `SetOnConnect` | `void` | `std::function<void(Connection*)> callback` | Set connect callback |
| `SetOnDisconnect` | `void` | `std::function<void(Connection*)> callback` | Set disconnect callback |
| `SetOnReceive` | `void` | `std::function<void(Connection*, Packet&)> callback` | Set receive callback |

---

## `network/packet.h`

### `enum class PacketType : uint8_t`
| Value | Description |
|-------|-------------|
| `Data` | Generic data |
| `Connect` | Connection request |
| `Disconnect` | Disconnect |
| `Ping` | Ping request |
| `Pong` | Ping response |
| `Ack` | Acknowledgment |
| `Reliable` | Reliable data |
| `Unreliable` | Unreliable data |
| `Ordered` | Ordered data |
| `Fragment` | Fragmented data |

### `class Packet`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Packet` | (ctor) | `void` | Constructor |
| `Packet` | (ctor) | `PacketType type` | Constructor with type |
| `GetType` | `PacketType` | `void` | Get packet type |
| `SetType` | `void` | `PacketType type` | Set packet type |
| `WriteInt8` | `void` | `int8_t value` | Write int8 |
| `WriteInt16` | `void` | `int16_t value` | Write int16 |
| `WriteInt32` | `void` | `int32_t value` | Write int32 |
| `WriteInt64` | `void` | `int64_t value` | Write int64 |
| `WriteUInt8` | `void` | `uint8_t value` | Write uint8 |
| `WriteUInt16` | `void` | `uint16_t value` | Write uint16 |
| `WriteUInt32` | `void` | `uint32_t value` | Write uint32 |
| `WriteUInt64` | `void` | `uint64_t value` | Write uint64 |
| `WriteFloat` | `void` | `float value` | Write float |
| `WriteDouble` | `void` | `double value` | Write double |
| `WriteBool` | `void` | `bool value` | Write bool |
| `WriteString` | `void` | `const std::string& value` | Write string |
| `WriteVec2` | `void` | `const Vec2& value` | Write Vec2 |
| `WriteVec3` | `void` | `const Vec3& value` | Write Vec3 |
| `WriteBytes` | `void` | `const void* data, size_t size` | Write raw bytes |
| `ReadInt8` | `int8_t` | `void` | Read int8 |
| `ReadInt16` | `int16_t` | `void` | Read int16 |
| `ReadInt32` | `int32_t` | `void` | Read int32 |
| `ReadInt64` | `int64_t` | `void` | Read int64 |
| `ReadUInt8` | `uint8_t` | `void` | Read uint8 |
| `ReadUInt16` | `uint16_t` | `void` | Read uint16 |
| `ReadUInt32` | `uint32_t` | `void` | Read uint32 |
| `ReadUInt64` | `uint64_t` | `void` | Read uint64 |
| `ReadFloat` | `float` | `void` | Read float |
| `ReadDouble` | `double` | `void` | Read double |
| `ReadBool` | `bool` | `void` | Read bool |
| `ReadString` | `std::string` | `void` | Read string |
| `ReadVec2` | `Vec2` | `void` | Read Vec2 |
| `ReadVec3` | `Vec3` | `void` | Read Vec3 |
| `ReadBytes` | `size_t` | `void* data, size_t size` | Read raw bytes |
| `GetSize` | `size_t` | `void` | Get packet size |
| `GetData` | `const uint8_t*` | `void` | Get raw data |
| `Clear` | `void` | `void` | Clear packet |
| `IsEmpty` | `bool` | `void` | Check empty |
| `SetReliable` | `void` | `bool reliable` | Set reliable |
| `IsReliable` | `bool` | `void` | Check reliable |
| `SetOrdered` | `void` | `bool ordered` | Set ordered |
| `IsOrdered` | `bool` | `void` | Check ordered |
| `SetChannel` | `void` | `uint8_t channel` | Set channel |
| `GetChannel` | `uint8_t` | `void` | Get channel |
| `SetTimestamp` | `void` | `double timestamp` | Set timestamp |
| `GetTimestamp` | `double` | `void` | Get timestamp |
| `Clone` | `Packet` | `void` | Create copy |

---

## `network/connection.h`

### `enum class ConnectionState`
| Value | Description |
|-------|-------------|
| `Connecting` | Connecting |
| `Connected` | Connected |
| `Disconnecting` | Disconnecting |
| `Disconnected` | Disconnected |

### `struct ConnectionStats`
| Member | Type | Description |
|--------|------|-------------|
| `ping` | `float` | Ping in ms |
| `packetsSent` | `uint64_t` | Packets sent |
| `packetsReceived` | `uint64_t` | Packets received |
| `bytesSent` | `uint64_t` | Bytes sent |
| `bytesReceived` | `uint64_t` | Bytes received |
| `packetLoss` | `float` | Packet loss rate |
| `bandwidthOut` | `float` | Outgoing bandwidth |
| `bandwidthIn` | `float` | Incoming bandwidth |

### `class Connection`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Connection` | (ctor) | `void` | Constructor |
| `GetID` | `ConnectionID` | `void` | Get connection ID |
| `GetState` | `ConnectionState` | `void` | Get state |
| `GetAddress` | `std::string` | `void` | Get remote address |
| `GetPort` | `uint16_t` | `void` | Get remote port |
| `GetPing` | `float` | `void` | Get ping |
| `GetStats` | `ConnectionStats` | `void` | Get stats |
| `Send` | `void` | `const Packet& packet` | Send packet |
| `Receive` | `bool` | `Packet& packet` | Receive packet |
| `Disconnect` | `void` | `void` | Disconnect |
| `IsConnected` | `bool` | `void` | Check connected |
| `SetPlayerName` | `void` | `const std::string& name` | Set player name |
| `GetPlayerName` | `std::string` | `void` | Get player name |
| `SetPlayerID` | `void` | `uint32_t id` | Set player ID |
| `GetPlayerID` | `uint32_t` | `void` | Get player ID |
| `SetLatency` | `void` | `float latency` | Set simulated latency |
| `GetLatency` | `float` | `void` | Get simulated latency |
| `SetPacketLoss` | `void` | `float loss` | Set simulated packet loss |
| `GetPacketLoss` | `float` | `void` | Get simulated packet loss |
| `GetTimeSinceLastPacket` | `float` | `void` | Time since last packet |
| `SetTimeout` | `void` | `float timeout` | Set timeout |
| `GetTimeout` | `float` | `void` | Get timeout |
