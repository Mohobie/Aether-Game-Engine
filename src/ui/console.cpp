#include "ui/console.h"
#include "core/logger.h"
#include <iostream>
#include <sstream>

// Stub implementation
namespace vge {

Console::Console() : visible(false), maxLines(100) {}

Console::~Console() {}

void Console::Initialize() {
    std::cout << "[Console] Initialized" << std::endl;
}

void Console::Shutdown() {
    lines.clear();
}

void Console::AddLine(const std::string& line) {
    lines.push_back(line);
    if (lines.size() > maxLines) {
        lines.erase(lines.begin());
    }
}

void Console::Clear() {
    lines.clear();
}

void Console::Render() {
    if (!visible) return;
    
    // Would render ImGui console window
    std::cout << "=== Console ===" << std::endl;
    for (const auto& line : lines) {
        std::cout << line << std::endl;
    }
}

void Console::Toggle() {
    visible = !visible;
}

void Console::ExecuteCommand(const std::string& cmd) {
    AddLine("> " + cmd);
    
    // Simple command parsing
    if (cmd == "help") {
        AddLine("Available commands: help, clear, quit");
    } else if (cmd == "clear") {
        Clear();
    } else if (cmd == "quit") {
        AddLine("Quitting...");
    } else {
        AddLine("Unknown command: " + cmd);
    }
}

} // namespace vge
