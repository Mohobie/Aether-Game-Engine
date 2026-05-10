#pragma once
#include "math/vec3.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <queue>

namespace vge {

// ============================================
// Network Protocol
// ============================================
enum class PacketType : uint8_t {
    // Connection
    Handshake = 0,
    Disconnect = 1,
    Heartbeat = 2,
    
    // World State
    WorldSnapshot = 10,
    EntityDelta = 11,
    ChunkData = 12,
    BlockUpdate = 13,
    
    // Player
    PlayerInput = 20,
    PlayerState = 21,
    PlayerJoin = 22,
    PlayerLeave = 23,
    
    // Physics
    PhysicsState = 30,
    PhysicsEvent = 31,
    
    // RPC
    RPCCall = 40,
    RPCResponse = 41,
    
    // Chat
    ChatMessage = 50,
    
    // Custom
    Custom = 100
};

// ============================================
// Packet Structure
// ============================================
struct Packet {
    PacketType type;
    uint32_t sequence;
    uint32_t timestamp;
    uint16_t payloadSize;
    std::vector<uint8_t> payload;
    
    Packet() : type(PacketType::Custom), sequence(0), timestamp(0), payloadSize(0) {}
    
    // Serialization
    std::vector<uint8_t> Serialize() const;
    bool Deserialize(const std::vector<uint8_t>& data);
};

// ============================================
// Network Connection
// ============================================
enum class ConnectionState {
    Disconnected,
    Connecting,
    Connected,
    Disconnecting
};

class NetworkConnection {
private:
    int connectionId;
    ConnectionState state;
    std::string address;
    int port;
    
    // Reliability
    uint32_t localSequence;
    uint32_t remoteSequence;
    std::unordered_map<uint32_t, Packet> sentPackets;
    std::queue<Packet> pendingPackets;
    
    // Timing
    float lastReceiveTime;
    float rtt; // Round-trip time
    float packetLoss;
    
public:
    NetworkConnection(int id, const std::string& addr, int port);
    ~NetworkConnection();
    
    // State
    void SetState(ConnectionState newState);
    ConnectionState GetState() const { return state; }
    int GetId() const { return connectionId; }
    
    // Send/Receive
    void SendPacket(const Packet& packet);
    void ReceivePacket(const Packet& packet);
    
    // Reliability
    void AcknowledgePacket(uint32_t sequence);
    void ResendUnacknowledged();
    
    // Timing
    void Update(float deltaTime);
    float GetRTT() const { return rtt; }
    float GetPacketLoss() const { return packetLoss; }
    
    // Disconnection
    void Disconnect();
    bool IsTimedOut() const;
};

// ============================================
// Server
// ============================================
class NetworkServer {
private:
    int port;
    bool running;
    
    std::unordered_map<int, std::unique_ptr<NetworkConnection>> connections;
    int nextConnectionId;
    
    // Callbacks
    std::function<void(int)> onClientConnected;
    std::function<void(int)> onClientDisconnected;
    std::function<void(int, const Packet&)> onPacketReceived;
    
    // State replication
public:
    struct EntityState {
        uint32_t entityId;
        Vec3 position;
        Vec3 rotation;
        Vec3 velocity;
        std::vector<uint8_t> customData;
        uint32_t lastUpdateTime;
    };
    
private:
    std::unordered_map<uint32_t, EntityState> entityStates;
    float snapshotInterval;
    float snapshotTimer;
    
public:
    NetworkServer(int port = 7777);
    ~NetworkServer();
    
    // Lifecycle
    bool Start();
    void Stop();
    bool IsRunning() const { return running; }
    
    // Connections
    void DisconnectClient(int connectionId);
    void DisconnectAll();
    int GetClientCount() const { return connections.size(); }
    std::vector<int> GetClientIds() const;
    
    // Send
    void Broadcast(const Packet& packet);
    void SendTo(int connectionId, const Packet& packet);
    void SendToAllExcept(int connectionId, const Packet& packet);
    
    // Receive
    void Update(float deltaTime);
    void ProcessPacket(int connectionId, const Packet& packet);
    
    // State Replication
    void SetSnapshotInterval(float interval) { snapshotInterval = interval; }
    void ReplicateEntity(uint32_t entityId, const Vec3& position, const Vec3& rotation, const Vec3& velocity);
    void RemoveEntity(uint32_t entityId);
    
    // Callbacks
    void SetOnClientConnected(std::function<void(int)> callback) { onClientConnected = callback; }
    void SetOnClientDisconnected(std::function<void(int)> callback) { onClientDisconnected = callback; }
    void SetOnPacketReceived(std::function<void(int, const Packet&)> callback) { onPacketReceived = callback; }
    
    // RPC
    void RegisterRPC(const std::string& name, std::function<void(int, const std::vector<uint8_t>&)> handler);
    void CallClientRPC(int connectionId, const std::string& name, const std::vector<uint8_t>& args);
    void CallAllClientsRPC(const std::string& name, const std::vector<uint8_t>& args);
};

// ============================================
// Client
// ============================================
class NetworkClient {
private:
    std::string serverAddress;
    int serverPort;
    ConnectionState state;
    
    std::unique_ptr<NetworkConnection> connection;
    
    // Callbacks
    std::function<void()> onConnected;
    std::function<void()> onDisconnected;
    std::function<void(const Packet&)> onPacketReceived;
    std::function<void(float)> onLatencyUpdated;
    
    // Prediction
    Vec3 predictedPosition;
    Vec3 predictedVelocity;
    std::queue<std::pair<uint32_t, Vec3>> inputHistory;
    
    // Interpolation
    struct Snapshot {
        uint32_t timestamp;
        std::unordered_map<uint32_t, NetworkServer::EntityState> entities;
    };
    
    std::queue<Snapshot> snapshotBuffer;
    float interpolationDelay;
    
public:
    NetworkClient();
    ~NetworkClient();
    
    // Connection
    bool Connect(const std::string& address, int port);
    void Disconnect();
    bool IsConnected() const { return state == ConnectionState::Connected; }
    ConnectionState GetState() const { return state; }
    
    // Send/Receive
    void SendPacket(const Packet& packet);
    void Update(float deltaTime);
    void ProcessPacket(const Packet& packet);
    
    // Input
    void SendPlayerInput(const Vec3& movement, const Vec3& rotation, bool jump, bool attack);
    
    // Prediction
    void SetPredictedPosition(const Vec3& pos) { predictedPosition = pos; }
    Vec3 GetPredictedPosition() const { return predictedPosition; }
    void ReconcilePrediction(uint32_t serverTimestamp, const Vec3& serverPosition);
    
    // Interpolation
    void SetInterpolationDelay(float delay) { interpolationDelay = delay; }
    float GetInterpolationDelay() const { return interpolationDelay; }
    
    // Callbacks
    void SetOnConnected(std::function<void()> callback) { onConnected = callback; }
    void SetOnDisconnected(std::function<void()> callback) { onDisconnected = callback; }
    void SetOnPacketReceived(std::function<void(const Packet&)> callback) { onPacketReceived = callback; }
    void SetOnLatencyUpdated(std::function<void(float)> callback) { onLatencyUpdated = callback; }
    
    // RPC
    void RegisterRPC(const std::string& name, std::function<void(const std::vector<uint8_t>&)> handler);
    void CallServerRPC(const std::string& name, const std::vector<uint8_t>& args);
};

// ============================================
// Lag Compensation
// ============================================
class LagCompensation {
private:
    struct HistoricalState {
        uint32_t timestamp;
        Vec3 position;
        Vec3 rotation;
        Vec3 velocity;
    };
    
    std::unordered_map<uint32_t, std::vector<HistoricalState>> history;
    float historyDuration;
    
public:
    LagCompensation(float historyDuration = 1.0f);
    
    // Record state
    void RecordState(uint32_t entityId, uint32_t timestamp, const Vec3& position, 
                     const Vec3& rotation, const Vec3& velocity);
    
    // Rewind to specific time
    bool GetHistoricalState(uint32_t entityId, uint32_t timestamp, 
                           Vec3& outPosition, Vec3& outRotation);
    
    // Clean old history
    void Update(float deltaTime);
};

// ============================================
// Network Manager (Singleton)
// ============================================
class NetworkManager {
private:
    static NetworkManager* instance;
    
    std::unique_ptr<NetworkServer> server;
    std::unique_ptr<NetworkClient> client;
    bool isServer;
    bool isClient;
    
    NetworkManager();
    
public:
    static NetworkManager* GetInstance();
    
    // Server mode
    bool StartServer(int port = 7777);
    void StopServer();
    NetworkServer* GetServer() const { return server.get(); }
    
    // Client mode
    bool ConnectToServer(const std::string& address, int port = 7777);
    void DisconnectFromServer();
    NetworkClient* GetClient() const { return client.get(); }
    
    // Mode checks
    bool IsServer() const { return isServer; }
    bool IsClient() const { return isClient; }
    bool IsMultiplayer() const { return isServer || isClient; }
    
    // Update
    void Update(float deltaTime);
    
    // Utility
    static uint32_t GetTimestamp();
    static float CalculateInterpolationFactor(uint32_t past, uint32_t present, uint32_t target);
};

} // namespace vge