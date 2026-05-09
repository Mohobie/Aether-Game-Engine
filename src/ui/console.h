#pragma once
#include <string>
#include <vector>

namespace vge {
    class Console {
    private:
        std::vector<std::string> lines;
        bool visible;
        int maxLines;
        
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
    };
}