#pragma once
#include <string>

namespace vge {
class ImGuiWrapper {
public:
    bool Initialize(void* window);
    void Shutdown();
    void NewFrame();
    void Render();
    void Text(const std::string& text);
    bool Button(const std::string& label);
};
}