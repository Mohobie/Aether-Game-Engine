#include "imgui_wrapper.h"
#include <iostream>

namespace vge {
bool ImGuiWrapper::Initialize(void* window) {
    std::cout << "[ImGui] Initialized (stub)" << std::endl;
    (void)window;
    return true;
}

void ImGuiWrapper::Shutdown() {
    std::cout << "[ImGui] Shutdown" << std::endl;
}

void ImGuiWrapper::NewFrame() {
    // Would call ImGui::NewFrame()
}

void ImGuiWrapper::Render() {
    // Would call ImGui::Render()
}

void ImGuiWrapper::Text(const std::string& text) {
    std::cout << "[UI] " << text << std::endl;
}

bool ImGuiWrapper::Button(const std::string& label) {
    std::cout << "[UI] Button: " << label << std::endl;
    return false;
}
}