#pragma once
#include <string>
#include <functional>

namespace vge {
    class NetworkClient {
    public:
        void Connect(const std::string& address, int port);
        void Disconnect();
        void SendMessage(const std::string& msg);
        void OnMessageReceived(std::function<void(const std::string&)> callback);
        bool IsConnected() const;
    private:
        bool connected = false;
    };
}
