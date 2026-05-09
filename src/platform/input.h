#pragma once
#include <GLFW/glfw3.h>
#include <unordered_map>

namespace vge {
class Input {
    std::unordered_map<int, bool> keys;
    std::unordered_map<int, bool> mouseButtons;
    double mouseX, mouseY;
public:
    void Update(GLFWwindow* window);
    bool IsKeyPressed(int key) const;
    bool IsMousePressed(int button) const;
    void GetMousePos(double& x, double& y) const;
};
}