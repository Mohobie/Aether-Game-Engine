#pragma once
#include "network_serialization.h"
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

namespace vge {

// Forward declarations
class NetworkServer2;
class NetworkClient2;

// RPC Callback types
using ServerRPCCallback = std::function<void(ClientID sender, RPCArgs& args)>;
using ClientRPCCallback = std::function<void(RPCArgs& args)>;

// ============================================
// Network Server (Host)
// ============================================
class NetworkServer2 {
public:
    NetworkServer2(uint16_t listenPort);
    ~NetworkServer2();

    // Lifecycle
    bool Start();
    void Stop();
    bool IsRunning() const { return running; }

    // Client management
    void DisconnectClient(ClientID clientId);
    size_t GetClientCount() const { return clients.size(); }

    // Event callbacks
    void OnClientConnected(std::function<void(ClientID)> callback) { onClientConnected = callback; }
    void OnClientDisconnected(std::function<void(ClientID)> callback) { onClientDisconnected = callback; }

    // RPC System
    void RegisterRPC(const std::string& name, ServerRPCCallback callback);
    void CallRPC(ClientID clientId, const std::string& name, RPCArgs& args);
    void BroadcastRPC(const std::string& name, RPCArgs& args);

    // Message sending
    void SendReliable(ClientID clientId, const std::vector<uint8_t>& data);
    void SendUnreliable(ClientID clientId, const std::vector<uint8_t>& data);
    void BroadcastReliable(const std::vector<uint8_t>& data);
    void BroadcastUnreliable(const std::vector<uint8_t>& data);

    // Update - process incoming messages
    void Update(float deltaTime);

    uint16_t GetPort() const { return port; }

private:
    uint16_t port;
    bool running;
    ClientID nextClientId;

    std::unordered_map<ClientID, bool> clients;
    std::unordered_map<std::string, ServerRPCCallback> rpcHandlers;

    std::function<void(ClientID)> onClientConnected;
    std::function<void(ClientID)> onClientDisconnected;

    // Simulated message queues for testing
    std::vector<std::pair<ClientID, std::vector<uint8_t>>> reliableQueue;
    std::vector<std::pair<ClientID, std::vector<uint8_t>>> unreliableQueue;
};

// ============================================
// Network Client
// ============================================
class NetworkClient2 {
public:
    NetworkClient2();
    ~NetworkClient2();

    // Connection
    bool Connect(const std::string& address, uint16_t port);
    void Disconnect();
    bool IsConnected() const { return connected; }

    // Event callbacks
    void OnConnected(std::function<void()> callback) { onConnected = callback; }
    void OnDisconnected(std::function<void()> callback) { onDisconnected = callback; }

    // RPC System
    void RegisterRPC(const std::string& name, ClientRPCCallback callback);
    void CallRPC(const std::string& name, RPCArgs& args);

    // Message sending
    void SendReliable(const std::vector<uint8_t>& data);
    void SendUnreliable(const std::vector<uint8_t>& data);

    // Update - process incoming messages
    void Update(float deltaTime);

    ClientID GetClientId() const { return clientId; }

private:
    std::string serverAddress;
    uint16_t serverPort;
    bool connected;
    ClientID clientId;

    std::unordered_map<std::string, ClientRPCCallback> rpcHandlers;

    std::function<void()> onConnected;
    std::function<void()> onDisconnected;

    // Simulated message queues for testing
    std::vector<std::vector<uint8_t>> incomingReliable;
    std::vector<std::vector<uint8_t>> incomingUnreliable;
};

// ============================================
// Network Manager (Factory)
// ============================================
class NetworkManager2 {
public:
    NetworkManager2();
    ~NetworkManager2();

    // Factory methods
    NetworkServer2* CreateServer(uint16_t port);
    NetworkClient2* CreateClient();

    // Update all connections
    void Update(float deltaTime);

    // Cleanup
    void Shutdown();

private:
    std::vector<std::unique_ptr<NetworkServer2>> servers;
    std::vector<std::unique_ptr<NetworkClient2>> clients;
};

} // namespace vge
