#include "ui/imgui_wrapper.h"
#include "platform/window.h"
#include <iostream>

// Stub implementation when Dear ImGui is not available
namespace vge {

ImGuiWrapper::ImGuiWrapper() : initialized(false) {}

ImGuiWrapper::~ImGuiWrapper() {
    if (initialized) Shutdown();
}

bool ImGuiWrapper::Initialize(void* windowHandle) {
    std::cout << "[ImGui] Stub - would initialize Dear ImGui" << std::endl;
    initialized = true;
    return true;
}

void ImGuiWrapper::Shutdown() {
    std::cout << "[ImGui] Stub - shutting down" << std::endl;
    initialized = false;
}

void ImGuiWrapper::BeginFrame() {
    // Stub - would start ImGui frame
}

void ImGuiWrapper::EndFrame() {
    // Stub - would render ImGui
}

void ImGuiWrapper::Render() {
    // Stub - would render all ImGui windows
}

} // namespace vge