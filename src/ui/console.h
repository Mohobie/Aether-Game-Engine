#pragma once
#include <string>
#include <vector>

namespace vge {

    // Forward declarations
    class CommandExecutor;
    struct CommandContext;

    class Console {
    private:
        std::vector<std::string> lines;
        bool visible;
        int maxLines;
        CommandExecutor* executor;
        
    public:
        Console();
        ~Console();
        
        void Initialize();
        void Shutdown();
        
        void AddLine(const std::string& line);
        void Clear();
        void Render();
        void Toggle();
        
        void ExecuteCommand(const std::string& cmd);
        bool IsVisible() const { return visible; }
        
        // Command system integration
        void SetupCommands(const CommandContext& ctx);
        std::string GetHistoryText() const;
        void SetCheatsEnabled(bool enabled);
        void SetDebugEnabled(bool enabled);
    };
}