#include "ui/console.h"
#include "ui/console_commands.h"
#include "core/logger.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace vge {

Console::Console() : visible(false), maxLines(100), executor(nullptr) {}

Console::~Console() {
    Shutdown();
}

void Console::Initialize() {
    std::cout << "[Console] Initialized" << std::endl;
    executor = new CommandExecutor();
}

void Console::Shutdown() {
    if (executor) {
        executor->Shutdown();
        delete executor;
        executor = nullptr;
    }
    lines.clear();
}

void Console::SetupCommands(const CommandContext& ctx) {
    if (executor) {
        executor->Initialize(ctx);
        AddLine("Console commands initialized. Type 'help' for available commands.");
    }
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
    
    if (!executor) {
        AddLine("Command executor not initialized");
        return;
    }
    
    std::string output;
    CommandResult result = executor->Execute(cmd, output);
    
    if (output == "__CLEAR__") {
        Clear();
        return;
    }
    
    if (!output.empty()) {
        // Split multi-line output
        std::stringstream ss(output);
        std::string line;
        while (std::getline(ss, line)) {
            if (!line.empty()) {
                AddLine(line);
            }
        }
    }
    
    if (result != CommandResult::Success && output.empty()) {
        switch (result) {
            case CommandResult::InvalidCommand:
                AddLine("Unknown command. Type 'help' for available commands.");
                break;
            case CommandResult::InvalidArguments:
                AddLine("Invalid arguments. Check command usage.");
                break;
            case CommandResult::ExecutionError:
                AddLine("Command execution failed.");
                break;
            default:
                AddLine("Command failed.");
                break;
        }
    }
}

std::string Console::GetHistoryText() const {
    if (!executor) return "";
    
    std::stringstream ss;
    const auto& history = executor->GetHistory();
    for (size_t i = 0; i < history.size(); ++i) {
        ss << i + 1 << ". " << history[i] << "\n";
    }
    return ss.str();
}

void Console::SetCheatsEnabled(bool enabled) {
    if (executor) {
        executor->SetCheatsEnabled(enabled);
        AddLine(enabled ? "Cheats enabled." : "Cheats disabled.");
    }
}

void Console::SetDebugEnabled(bool enabled) {
    if (executor) {
        executor->SetDebugEnabled(enabled);
        AddLine(enabled ? "Debug mode enabled." : "Debug mode disabled.");
    }
}

} // namespace vge
