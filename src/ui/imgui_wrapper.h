#pragma once

namespace vge {

class ImGuiWrapper {
private:
    bool initialized;
    
public:
    ImGuiWrapper();
    ~ImGuiWrapper();
    
    bool Initialize(void* windowHandle);
    void Shutdown();
    
    void BeginFrame();
    void EndFrame();
    void Render();
};

} // namespace vge
