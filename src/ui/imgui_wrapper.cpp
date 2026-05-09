#include "ui/imgui_wrapper.h"
#include "platform/window.h"
#include <iostream>

// Software-based UI rendering (no Dear ImGui required)
// Uses terminal output for menus

namespace vge {

ImGuiWrapper::ImGuiWrapper() : initialized(false) {}

ImGuiWrapper::~ImGuiWrapper() {
    if (initialized) Shutdown();
}

bool ImGuiWrapper::Initialize(void* windowHandle) {
    std::cout << "[ImGui] Software UI initialized (terminal mode)" << std::endl;
    initialized = true;
    return true;
}

void ImGuiWrapper::Shutdown() {
    std::cout << "[ImGui] Software UI shutdown" << std::endl;
    initialized = false;
}

void ImGuiWrapper::BeginFrame() {
    // Clear terminal for fresh frame
    std::cout << "\033[2J\033[H";
}

void ImGuiWrapper::EndFrame() {
    // Frame complete - nothing to do for terminal
}

void ImGuiWrapper::Render() {
    // Would render all UI windows
    // For now, just placeholder
}

} // namespace vge