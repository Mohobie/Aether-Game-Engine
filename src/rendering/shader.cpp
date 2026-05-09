#include "rendering/shader.h"
#include <iostream>
#include <fstream>
#include <sstream>

// Stub implementation when OpenGL is not available
namespace vge {

Shader::Shader() : programId(0), initialized(false) {}

Shader::~Shader() {
    if (initialized) {
        // Would delete OpenGL program
    }
}

bool Shader::LoadFromFile(const std::string& vertexPath, const std::string& fragmentPath) {
    std::cout << "[Shader] Stub - would load " << vertexPath << " and " << fragmentPath << std::endl;
    initialized = true;
    return true;
}

bool Shader::LoadFromSource(const std::string& vertexSource, const std::string& fragmentSource) {
    std::cout << "[Shader] Stub - would compile shaders from source" << std::endl;
    initialized = true;
    return true;
}

void Shader::Bind() const {
    // Stub - would use OpenGL program
}

void Shader::SetInt(const std::string& name, int value) const {
    // Stub
}

void Shader::SetFloat(const std::string& name, float value) const {
    // Stub
}

void Shader::SetVec3(const std::string& name, const Vec3& value) const {
    // Stub
}

void Shader::SetMat4(const std::string& name, const Mat4& value) const {
    // Stub
}

} // namespace vge