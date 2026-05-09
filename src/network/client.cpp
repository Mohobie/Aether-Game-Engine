#include "client.h"
#include <iostream>

namespace vge {

NetworkClient::NetworkClient() : connected(false), serverAddress(""), serverPort(7777) {}

NetworkClient::~NetworkClient() {
    if (connected) Disconnect();
}

bool NetworkClient::Connect(const std::string& address, int port) {
    serverAddress = address;
    serverPort = port;
    std::cout << "[Client] Connecting to " << address << ":" << port << " (stub)" << std::endl;
    connected = true;
    return true;
}

void NetworkClient::Disconnect() {
    std::cout << "[Client] Disconnecting" << std::endl;
    connected = false;
}

void NetworkClient::SendPacket(const Packet& packet) {
    // Stub: Would serialize and send packet
}

void NetworkClient::Update() {
    // Stub: Would receive and process packets
}

bool NetworkClient::IsConnected() const {
    return connected;
}

} // namespace vge
