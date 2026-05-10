#include "network_architecture.h"
#include <iostream>
#include <cstring>
#include <chrono>

namespace vge {

// ============================================
// Packet Implementation
// ============================================

std::vector<uint8_t> Packet::Serialize() const {
    std::vector<uint8_t> data;
    
    // Header: type (1) + sequence (4) + timestamp (4) + payloadSize (2)
    size_t headerSize = 1 + 4 + 4 + 2;
    data.reserve(headerSize + payload.size());
    
    // Type
    data.push_back(static_cast<uint8_t>(type));
    
    // Sequence (big-endian)
    data.push_back((sequence >> 24) & 0xFF);
    data.push_back((sequence >> 16) & 0xFF);
    data.push_back((sequence >> 8) & 0xFF);
    data.push_back(sequence & 0xFF);
    
    // Timestamp
    data.push_back((timestamp >> 24) & 0xFF);
    data.push_back((timestamp >> 16) & 0xFF);
    data.push_back((timestamp >> 8) & 0xFF);
    data.push_back(timestamp & 0xFF);
    
    // Payload size
    data.push_back((payloadSize >> 8) & 0xFF);
    data.push_back(payloadSize & 0xFF);
    
    // Payload
    data.insert(data.end(), payload.begin(), payload.end());
    
    return data;
}

bool Packet::Deserialize(const std::vector<uint8_t>& data) {
    if (data.size() < 11) return false;
    
    size_t offset = 0;
    
    // Type
    type = static_cast<PacketType>(data[offset++]);
    
    // Sequence
    sequence = (data[offset] << 24) | (data[offset + 1] << 16) | 
               (data[offset + 2] << 8) | data[offset + 3];
    offset += 4;
    
    // Timestamp
    timestamp = (data[offset] << 24) | (data[offset + 1] << 16) | 
                (data[offset + 2] << 8) | data[offset + 3];
    offset += 4;
    
    // Payload size
    payloadSize = (data[offset] << 8) | data[offset + 1];
    offset += 2;
    
    // Payload
    if (data.size() >= offset + payloadSize) {
        payload.assign(data.begin() + offset, data.begin() + offset + payloadSize);
        return true;
    }
    
    return false;
}

// ============================================
// Network Connection
// ============================================

NetworkConnection::NetworkConnection(int id, const std::string& addr, int port)
    : connectionId(id)
    , state(ConnectionState::Disconnected)
    , address(addr)
    , port(port)
    , localSequence(0)
    , remoteSequence(0)
    , lastReceiveTime(0.0f)
    , rtt(0.0f)
    , packetLoss(0.0f) {}

NetworkConnection::~NetworkConnection() {}

void NetworkConnection::SetState(ConnectionState newState) {
    state = newState;
    std::cout << "[NetworkConnection " << connectionId << "] State: " 
              << static_cast<int>(state) << std::endl;
}

void NetworkConnection::SendPacket(const Packet& packet) {
    Packet sentPacket = packet;
    sentPacket.sequence = localSequence++;
    sentPacket.timestamp = NetworkManager::GetTimestamp();
    
    sentPackets[sentPacket.sequence] = sentPacket;
    pendingPackets.push(sentPacket);
    
    std::cout << "[NetworkConnection " << connectionId << "] Sending packet " 
              << sentPacket.sequence << " type " << static_cast<int>(sentPacket.type) << std::endl;
}

void NetworkConnection::ReceivePacket(const Packet& packet) {
    lastReceiveTime = 0.0f; // Reset timeout
    
    // Calculate RTT
    uint32_t currentTime = NetworkManager::GetTimestamp();
    if (currentTime > packet.timestamp) {
        rtt = (currentTime - packet.timestamp) / 1000.0f;
    }
    
    std::cout << "[NetworkConnection " << connectionId << "] Received packet " 
              << packet.sequence << " type " << static_cast<int>(packet.type) << std::endl;
}

void NetworkConnection::AcknowledgePacket(uint32_t sequence) {
    auto it = sentPackets.find(sequence);
    if (it != sentPackets.end()) {
        sentPackets.erase(it);
        std::cout << "[NetworkConnection " << connectionId << "] Acknowledged packet " 
                  << sequence << std::endl;
    }
}

void NetworkConnection::ResendUnacknowledged() {
    // Resend packets that haven't been acknowledged
    for (const auto& [seq, packet] : sentPackets) {
        pendingPackets.push(packet);
        std::cout << "[NetworkConnection " << connectionId << "] Resending packet " 
                  << seq << std::endl;
    }
}

void NetworkConnection::Update(float deltaTime) {
    lastReceiveTime += deltaTime;
    
    // Update packet loss
    if (!sentPackets.empty()) {
        packetLoss = static_cast<float>(sentPackets.size()) / localSequence;
    }
}

void NetworkConnection::Disconnect() {
    SetState(ConnectionState::Disconnecting);
    
    // Send disconnect packet
    Packet disconnectPacket;
    disconnectPacket.type = PacketType::Disconnect;
    SendPacket(disconnectPacket);
    
    std::cout << "[NetworkConnection " << connectionId << "] Disconnecting" << std::endl;
}

bool NetworkConnection::IsTimedOut() const {
    return lastReceiveTime > 10.0f; // 10 second timeout
}

// ============================================
// Network Server
// ============================================

NetworkServer::NetworkServer(int port)
    : port(port)
    , running(false)
    , nextConnectionId(1)
    , snapshotInterval(0.05f) // 20 snapshots per second
    , snapshotTimer(0.0f) {}

NetworkServer::~NetworkServer() {
    Stop();
}

bool NetworkServer::Start() {
    if (running) return false;
    
    running = true;
    std::cout << "[NetworkServer] Started on port " << port << std::endl;
    
    return true;
}

void NetworkServer::Stop() {
    if (!running) return;
    
    DisconnectAll();
    running = false;
    
    std::cout << "[NetworkServer] Stopped" << std::endl;
}

void NetworkServer::DisconnectClient(int connectionId) {
    auto it = connections.find(connectionId);
    if (it != connections.end()) {
        it->second->Disconnect();
        connections.erase(it);
        
        if (onClientDisconnected) {
            onClientDisconnected(connectionId);
        }
        
        std::cout << "[NetworkServer] Client " << connectionId << " disconnected" << std::endl;
    }
}

void NetworkServer::DisconnectAll() {
    for (auto& [id, conn] : connections) {
        conn->Disconnect();
        if (onClientDisconnected) {
            onClientDisconnected(id);
        }
    }
    connections.clear();
}

std::vector<int> NetworkServer::GetClientIds() const {
    std::vector<int> ids;
    for (const auto& [id, conn] : connections) {
        ids.push_back(id);
    }
    return ids;
}

void NetworkServer::Broadcast(const Packet& packet) {
    for (auto& [id, conn] : connections) {
        conn->SendPacket(packet);
    }
}

void NetworkServer::SendTo(int connectionId, const Packet& packet) {
    auto it = connections.find(connectionId);
    if (it != connections.end()) {
        it->second->SendPacket(packet);
    }
}

void NetworkServer::SendToAllExcept(int connectionId, const Packet& packet) {
    for (auto& [id, conn] : connections) {
        if (id != connectionId) {
            conn->SendPacket(packet);
        }
    }
}

void NetworkServer::Update(float deltaTime) {
    if (!running) return;
    
    // Update connections
    for (auto& [id, conn] : connections) {
        conn->Update(deltaTime);
        
        // Check for timeouts
        if (conn->IsTimedOut()) {
            std::cout << "[NetworkServer] Client " << id << " timed out" << std::endl;
            DisconnectClient(id);
            break; // Iterator invalidated
        }
    }
    
    // Send snapshots
    snapshotTimer += deltaTime;
    if (snapshotTimer >= snapshotInterval) {
        snapshotTimer = 0.0f;
        
        // Build world snapshot
        Packet snapshot;
        snapshot.type = PacketType::WorldSnapshot;
        snapshot.timestamp = NetworkManager::GetTimestamp();
        
        // Serialize entity states
        for (const auto& statePair : entityStates) {
            uint32_t entityId = statePair.first;
            const EntityState& state = statePair.second;
            (void)entityId;
            (void)state;
            // Would serialize entity state into payload
        }
        
        Broadcast(snapshot);
    }
}

void NetworkServer::ProcessPacket(int connectionId, const Packet& packet) {
    auto it = connections.find(connectionId);
    if (it == connections.end()) return;
    
    it->second->ReceivePacket(packet);
    
    switch (packet.type) {
        case PacketType::Handshake:
            std::cout << "[NetworkServer] Client " << connectionId << " handshake received" << std::endl;
            break;
            
        case PacketType::Heartbeat:
            // Just update last receive time (done in ReceivePacket)
            break;
            
        case PacketType::PlayerInput: {
            // Process player input
            std::cout << "[NetworkServer] Player input from " << connectionId << std::endl;
            break;
        }
            
        case PacketType::RPCCall: {
            // Process RPC call
            std::cout << "[NetworkServer] RPC from " << connectionId << std::endl;
            break;
        }
            
        default:
            if (onPacketReceived) {
                onPacketReceived(connectionId, packet);
            }
            break;
    }
}

void NetworkServer::ReplicateEntity(uint32_t entityId, const Vec3& position, 
                                     const Vec3& rotation, const Vec3& velocity) {
    EntityState state;
    state.entityId = entityId;
    state.position = position;
    state.rotation = rotation;
    state.velocity = velocity;
    state.lastUpdateTime = NetworkManager::GetTimestamp();
    
    entityStates[entityId] = state;
}

void NetworkServer::RemoveEntity(uint32_t entityId) {
    entityStates.erase(entityId);
}

void NetworkServer::RegisterRPC(const std::string& name, 
                                std::function<void(int, const std::vector<uint8_t>&)> handler) {
    // Would store RPC handlers
    std::cout << "[NetworkServer] Registered RPC: " << name << std::endl;
    (void)handler;
}

void NetworkServer::CallClientRPC(int connectionId, const std::string& name, 
                                  const std::vector<uint8_t>& args) {
    Packet rpcPacket;
    rpcPacket.type = PacketType::RPCCall;
    // Would serialize RPC name and args into payload
    
    SendTo(connectionId, rpcPacket);
    std::cout << "[NetworkServer] Called RPC " << name << " on client " << connectionId << std::endl;
    (void)args;
}

void NetworkServer::CallAllClientsRPC(const std::string& name, const std::vector<uint8_t>& args) {
    Packet rpcPacket;
    rpcPacket.type = PacketType::RPCCall;
    // Would serialize RPC name and args into payload
    
    Broadcast(rpcPacket);
    std::cout << "[NetworkServer] Called RPC " << name << " on all clients" << std::endl;
    (void)args;
}

// ============================================
// Network Client
// ============================================

NetworkClient::NetworkClient()
    : serverAddress("")
    , serverPort(0)
    , state(ConnectionState::Disconnected)
    , interpolationDelay(0.1f) {}

NetworkClient::~NetworkClient() {
    Disconnect();
}

bool NetworkClient::Connect(const std::string& address, int port) {
    if (state == ConnectionState::Connected) {
        std::cout << "[NetworkClient] Already connected" << std::endl;
        return false;
    }
    
    serverAddress = address;
    serverPort = port;
    state = ConnectionState::Connecting;
    
    // Create connection
    connection = std::make_unique<NetworkConnection>(0, address, port);
    connection->SetState(ConnectionState::Connecting);
    
    // Send handshake
    Packet handshake;
    handshake.type = PacketType::Handshake;
    connection->SendPacket(handshake);
    
    std::cout << "[NetworkClient] Connecting to " << address << ":" << port << std::endl;
    
    // Simulate connection success
    state = ConnectionState::Connected;
    connection->SetState(ConnectionState::Connected);
    
    if (onConnected) {
        onConnected();
    }
    
    return true;
}

void NetworkClient::Disconnect() {
    if (state == ConnectionState::Disconnected) return;
    
    if (connection) {
        connection->Disconnect();
    }
    
    state = ConnectionState::Disconnected;
    
    if (onDisconnected) {
        onDisconnected();
    }
    
    std::cout << "[NetworkClient] Disconnected from server" << std::endl;
}

void NetworkClient::SendPacket(const Packet& packet) {
    if (connection && state == ConnectionState::Connected) {
        connection->SendPacket(packet);
    }
}

void NetworkClient::Update(float deltaTime) {
    if (connection) {
        connection->Update(deltaTime);
        
        // Check for timeout
        if (connection->IsTimedOut()) {
            std::cout << "[NetworkClient] Server connection timed out" << std::endl;
            Disconnect();
            return;
        }
        
        // Update latency callback
        if (onLatencyUpdated) {
            onLatencyUpdated(connection->GetRTT());
        }
    }
    
    // Process snapshot buffer for interpolation
    while (!snapshotBuffer.empty()) {
        Snapshot& snapshot = snapshotBuffer.front();
        uint32_t currentTime = NetworkManager::GetTimestamp();
        
        if (currentTime >= snapshot.timestamp + static_cast<uint32_t>(interpolationDelay * 1000)) {
            // Process snapshot
            std::cout << "[NetworkClient] Processing snapshot from " << snapshot.timestamp << std::endl;
            snapshotBuffer.pop();
        } else {
            break;
        }
    }
}

void NetworkClient::ProcessPacket(const Packet& packet) {
    if (connection) {
        connection->ReceivePacket(packet);
    }
    
    switch (packet.type) {
        case PacketType::WorldSnapshot: {
            // Store snapshot for interpolation
            Snapshot snapshot;
            snapshot.timestamp = packet.timestamp;
            // Would deserialize entities from payload
            snapshotBuffer.push(snapshot);
            break;
        }
            
        case PacketType::EntityDelta: {
            // Apply delta update
            std::cout << "[NetworkClient] Entity delta received" << std::endl;
            break;
        }
            
        case PacketType::PlayerState: {
            // Update player state
            std::cout << "[NetworkClient] Player state received" << std::endl;
            break;
        }
            
        case PacketType::RPCCall: {
            // Handle RPC call
            std::cout << "[NetworkClient] RPC received" << std::endl;
            break;
        }
            
        case PacketType::ChatMessage: {
            // Handle chat message
            std::cout << "[NetworkClient] Chat message received" << std::endl;
            break;
        }
            
        default:
            if (onPacketReceived) {
                onPacketReceived(packet);
            }
            break;
    }
}

void NetworkClient::SendPlayerInput(const Vec3& movement, const Vec3& rotation, bool jump, bool attack) {
    Packet inputPacket;
    inputPacket.type = PacketType::PlayerInput;
    
    // Serialize input
    // Would pack movement, rotation, jump, attack into payload
    
    SendPacket(inputPacket);
    (void)movement; (void)rotation; (void)jump; (void)attack;
}

void NetworkClient::ReconcilePrediction(uint32_t serverTimestamp, const Vec3& serverPosition) {
    // Remove acknowledged inputs from history
    while (!inputHistory.empty() && inputHistory.front().first <= serverTimestamp) {
        inputHistory.pop();
    }
    
    // If prediction is wrong, snap to server position
    float error = (predictedPosition - serverPosition).length();
    if (error > 0.1f) {
        std::cout << "[NetworkClient] Prediction error: " << error << ", snapping to server position" << std::endl;
        predictedPosition = serverPosition;
    }
}

void NetworkClient::RegisterRPC(const std::string& name, std::function<void(const std::vector<uint8_t>&)> handler) {
    // Would store RPC handlers
    std::cout << "[NetworkClient] Registered RPC: " << name << std::endl;
    (void)handler;
}

void NetworkClient::CallServerRPC(const std::string& name, const std::vector<uint8_t>& args) {
    Packet rpcPacket;
    rpcPacket.type = PacketType::RPCCall;
    // Would serialize RPC name and args into payload
    
    SendPacket(rpcPacket);
    std::cout << "[NetworkClient] Called RPC: " << name << std::endl;
    (void)args;
}

// ============================================
// Lag Compensation
// ============================================

LagCompensation::LagCompensation(float historyDuration)
    : historyDuration(historyDuration) {}

void LagCompensation::RecordState(uint32_t entityId, uint32_t timestamp, const Vec3& position,
                                   const Vec3& rotation, const Vec3& velocity) {
    HistoricalState state;
    state.timestamp = timestamp;
    state.position = position;
    state.rotation = rotation;
    state.velocity = velocity;
    
    history[entityId].push_back(state);
}

bool LagCompensation::GetHistoricalState(uint32_t entityId, uint32_t timestamp,
                                           Vec3& outPosition, Vec3& outRotation) {
    auto it = history.find(entityId);
    if (it == history.end() || it->second.empty()) {
        return false;
    }
    
    const auto& states = it->second;
    
    // Find states surrounding the target timestamp
    const HistoricalState* before = nullptr;
    const HistoricalState* after = nullptr;
    
    for (const auto& state : states) {
        if (state.timestamp <= timestamp) {
            before = &state;
        }
        if (state.timestamp >= timestamp && !after) {
            after = &state;
            break;
        }
    }
    
    if (!before && !after) {
        return false;
    }
    
    if (!before || !after || before == after) {
        // Use nearest state
        const HistoricalState* nearest = before ? before : after;
        outPosition = nearest->position;
        outRotation = nearest->rotation;
        return true;
    }
    
    // Interpolate between states
    float t = static_cast<float>(timestamp - before->timestamp) / 
               static_cast<float>(after->timestamp - before->timestamp);
    
    outPosition = before->position + (after->position - before->position) * t;
    outRotation = before->rotation + (after->rotation - before->rotation) * t;
    
    return true;
}

void LagCompensation::Update(float deltaTime) {
    uint32_t currentTime = NetworkManager::GetTimestamp();
    uint32_t maxAge = static_cast<uint32_t>(historyDuration * 1000);
    
    // Remove old history
    for (auto& [entityId, states] : history) {
        states.erase(
            std::remove_if(states.begin(), states.end(),
                [currentTime, maxAge](const HistoricalState& state) {
                    return (currentTime - state.timestamp) > maxAge;
                }),
            states.end()
        );
    }
    (void)deltaTime;
}

// ============================================
// Network Manager (Singleton)
// ============================================

NetworkManager* NetworkManager::instance = nullptr;

NetworkManager::NetworkManager()
    : isServer(false)
    , isClient(false) {}

NetworkManager* NetworkManager::GetInstance() {
    if (!instance) {
        instance = new NetworkManager();
    }
    return instance;
}

bool NetworkManager::StartServer(int port) {
    if (isClient) {
        std::cout << "[NetworkManager] Cannot start server while in client mode" << std::endl;
        return false;
    }
    
    server = std::make_unique<NetworkServer>(port);
    if (server->Start()) {
        isServer = true;
        std::cout << "[NetworkManager] Server mode started" << std::endl;
        return true;
    }
    
    return false;
}

void NetworkManager::StopServer() {
    if (server) {
        server->Stop();
        server.reset();
    }
    isServer = false;
    std::cout << "[NetworkManager] Server stopped" << std::endl;
}

bool NetworkManager::ConnectToServer(const std::string& address, int port) {
    if (isServer) {
        std::cout << "[NetworkManager] Cannot connect while in server mode" << std::endl;
        return false;
    }
    
    client = std::make_unique<NetworkClient>();
    if (client->Connect(address, port)) {
        isClient = true;
        std::cout << "[NetworkManager] Client mode started" << std::endl;
        return true;
    }
    
    return false;
}

void NetworkManager::DisconnectFromServer() {
    if (client) {
        client->Disconnect();
        client.reset();
    }
    isClient = false;
    std::cout << "[NetworkManager] Disconnected from server" << std::endl;
}

void NetworkManager::Update(float deltaTime) {
    if (server) {
        server->Update(deltaTime);
    }
    if (client) {
        client->Update(deltaTime);
    }
}

uint32_t NetworkManager::GetTimestamp() {
    auto now = std::chrono::steady_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    return static_cast<uint32_t>(ms);
}

float NetworkManager::CalculateInterpolationFactor(uint32_t past, uint32_t present, uint32_t target) {
    if (present == past) return 0.0f;
    
    float t = static_cast<float>(target - past) / static_cast<float>(present - past);
    return std::max(0.0f, std::min(1.0f, t));
}

} // namespace vge