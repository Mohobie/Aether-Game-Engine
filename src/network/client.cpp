#include "client.h"
#include <iostream>

namespace vge {

void NetworkClient::Connect(const std::string& address, int port) {
    std::cout << "[Client] Connecting to " << address << ":" << port << " (stub)" << std::endl;
    connected = true;
}

void NetworkClient::Disconnect() {
    std::cout << "[Client] Disconnecting" << std::endl;
    connected = false;
}

void NetworkClient::SendMessage(const std::string& msg) {
    std::cout << "[Client] Sending: " << msg << std::endl;
}

void NetworkClient::OnMessageReceived(std::function<void(const std::string&)> callback) {
    // Stub: Would set up message receive callback
}

bool NetworkClient::IsConnected() const {
    return connected;
}

} // namespace vge
