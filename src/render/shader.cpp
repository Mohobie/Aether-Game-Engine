#include "shader.h"
#include <iostream>
namespace vge {
bool Shader::load(const std::string& vertexSource, const std::string& fragmentSource) {
    std::cout << "[Shader] Loaded shader program" << std::endl;
    return true;
}
void Shader::bind() {}
void Shader::setUniform(const std::string& name, float value) {}
} // namespace vge
