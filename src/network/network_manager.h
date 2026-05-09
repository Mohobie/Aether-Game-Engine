#pragma once

#include <string>

namespace vge {
    class NetworkManager {
    public:
        void Initialize();
        void Connect(const std::string& address, int port);
        void Disconnect();
        void Send(const std::string& message);
        void Shutdown();
    };
}
