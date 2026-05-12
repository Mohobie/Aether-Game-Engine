#include "network_manager.h"
#include <iostream>

namespace vge {

// ============================================
// NetworkServer2 Implementation
// ============================================
NetworkServer2::NetworkServer2(uint16_t listenPort)
    : port(listenPort), running(false), nextClientId(1) {
}

NetworkServer2::~NetworkServer2() {
    Stop();
}

bool NetworkServer2::Start() {
    running = true;
    std::cout << "[NetworkServer] Started on port " << port << std::endl;
    return true;
}

void NetworkServer2::Stop() {
    running = false;
    clients.clear();
    std::cout << "[NetworkServer] Stopped" << std::endl;
}

void NetworkServer2::DisconnectClient(ClientID clientId) {
    clients.erase(clientId);
    if (onClientDisconnected) {
        onClientDisconnected(clientId);
    }
}

void NetworkServer2::RegisterRPC(const std::string& name, ServerRPCCallback callback) {
    rpcHandlers[name] = callback;
}

void NetworkServer2::CallRPC(ClientID clientId, const std::string& name, RPCArgs& args) {
    auto it = rpcHandlers.find(name);
    if (it != rpcHandlers.end()) {
        it->second(clientId, args);
    }
}

void NetworkServer2::BroadcastRPC(const std::string& name, RPCArgs& args) {
    for (auto& [clientId, _] : clients) {
        CallRPC(clientId, name, args);
    }
}

void NetworkServer2::SendReliable(ClientID clientId, const std::vector<uint8_t>& data) {
    reliableQueue.push_back({clientId, data});
}

void NetworkServer2::SendUnreliable(ClientID clientId, const std::vector<uint8_t>& data) {
    unreliableQueue.push_back({clientId, data});
}

void NetworkServer2::BroadcastReliable(const std::vector<uint8_t>& data) {
    for (auto& [clientId, _] : clients) {
        SendReliable(clientId, data);
    }
}

void NetworkServer2::BroadcastUnreliable(const std::vector<uint8_t>& data) {
    for (auto& [clientId, _] : clients) {
        SendUnreliable(clientId, data);
    }
}

void NetworkServer2::Update(float deltaTime) {
    // Process simulated connections (in real impl, would accept new connections)
    // For testing, we simulate clients connecting via the Connect method
}

// ============================================
// NetworkClient2 Implementation
// ============================================
NetworkClient2::NetworkClient2()
    : serverPort(0), connected(false), clientId(INVALID_CLIENT_ID) {
}

NetworkClient2::~NetworkClient2() {
    Disconnect();
}

bool NetworkClient2::Connect(const std::string& address, uint16_t port) {
    serverAddress = address;
    serverPort = port;
    connected = true;
    clientId = 1; // Simulated - in real impl, assigned by server

    std::cout << "[NetworkClient] Connected to " << address << ":" << port << std::endl;

    if (onConnected) {
        onConnected();
    }

    return true;
}

void NetworkClient2::Disconnect() {
    connected = false;
    clientId = INVALID_CLIENT_ID;

    if (onDisconnected) {
        onDisconnected();
    }
}

void NetworkClient2::RegisterRPC(const std::string& name, ClientRPCCallback callback) {
    rpcHandlers[name] = callback;
}

void NetworkClient2::CallRPC(const std::string& name, RPCArgs& args) {
    // In real impl, would serialize and send to server
    // For testing, we just log it
    std::cout << "[NetworkClient] Calling RPC: " << name << std::endl;
}

void NetworkClient2::SendReliable(const std::vector<uint8_t>& data) {
    // In real impl, would send over network
}

void NetworkClient2::SendUnreliable(const std::vector<uint8_t>& data) {
    // In real impl, would send over network
}

void NetworkClient2::Update(float deltaTime) {
    // Process incoming messages
}

// ============================================
// NetworkManager2 Implementation
// ============================================
NetworkManager2::NetworkManager2() {
}

NetworkManager2::~NetworkManager2() {
    Shutdown();
}

NetworkServer2* NetworkManager2::CreateServer(uint16_t port) {
    auto server = std::make_unique<NetworkServer2>(port);
    auto* ptr = server.get();
    servers.push_back(std::move(server));
    return ptr;
}

NetworkClient2* NetworkManager2::CreateClient() {
    auto client = std::make_unique<NetworkClient2>();
    auto* ptr = client.get();
    clients.push_back(std::move(client));
    return ptr;
}

void NetworkManager2::Update(float deltaTime) {
    for (auto& server : servers) {
        server->Update(deltaTime);
    }
    for (auto& client : clients) {
        client->Update(deltaTime);
    }
}

void NetworkManager2::Shutdown() {
    servers.clear();
    clients.clear();
}

} // namespace vge
