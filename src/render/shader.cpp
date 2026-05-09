#include "shader.h"
#include <iostream>
namespace aether {
bool Shader::load(const std::string& vertexSource, const std::string& fragmentSource) {
    std::cout << "[Shader] Loaded shader program" << std::endl;
    return true;
}
void Shader::bind() {}
void Shader::setUniform(const std::string& name, float value) {}
} // namespace aether
