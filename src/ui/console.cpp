#include "console.h"
#include <iostream>

namespace vge {
    void Console::Log(const std::string& msg) {
        history.push_back(msg);
        std::cout << "[CONSOLE] " << msg << std::endl;
    }
    
    void Console::ExecuteCommand(const std::string& cmd) {
        Log("> " + cmd);
        if (cmd == "clear") {
            Clear();
        } else if (cmd == "help") {
            Log("Available commands: clear, help, status");
        }
    }
    
    std::vector<std::string> Console::GetHistory() {
        return history;
    }
    
    void Console::Clear() {
        history.clear();
    }
}
