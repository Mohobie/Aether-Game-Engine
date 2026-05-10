#include "shader.h"
#include "platform/file_system.h"
#include <iostream>

namespace vge {

// ============================================
// Shader Implementation
// ============================================

Shader::Shader() : programID(0), loaded(false) {}

Shader::~Shader() {
    // Would delete OpenGL program
}

bool Shader::LoadFromSource(const std::string& vertexSource, const std::string& fragmentSource) {
    std::cout << "[Shader] Compiling shader program" << std::endl;
    // In a real implementation, this would compile GLSL shaders
    // For now, just mark as loaded
    loaded = true;
    return true;
}

bool Shader::LoadFromFile(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexSource = File::ReadText(vertexPath);
    std::string fragmentSource = File::ReadText(fragmentPath);
    
    if (vertexSource.empty() || fragmentSource.empty()) {
        std::cerr << "[Shader] Failed to load shader files" << std::endl;
        return false;
    }
    
    return LoadFromSource(vertexSource, fragmentSource);
}

void Shader::Bind() const {
    // Would call glUseProgram
}

void Shader::Unbind() const {
    // Would call glUseProgram(0)
}

void Shader::SetFloat(const std::string& name, float value) {
    // Would set uniform
}

void Shader::SetInt(const std::string& name, int value) {
    // Would set uniform
}

void Shader::SetVec3(const std::string& name, const Vec3& value) {
    // Would set uniform
}

void Shader::SetMat4(const std::string& name, const float* matrix) {
    // Would set uniform
}

// ============================================
// Default Shader Sources
// ============================================

std::string Shader::GetDefaultVertexShader() {
    return R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec2 aTexCoord;

out vec3 FragPos;
out vec3 Normal;
out vec3 Color;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    Color = aColor;
    TexCoord = aTexCoord;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";
}

std::string Shader::GetDefaultFragmentShader() {
    return R"(
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec3 Color;
in vec2 TexCoord;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 ambientLight;
uniform vec3 viewPos;
uniform sampler2D diffuseTexture;

void main() {
    // Ambient
    vec3 ambient = ambientLight * Color;
    
    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = lightColor * diff * Color;
    
    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = lightColor * spec * 0.5;
    
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
)";
}

std::string Shader::GetUnlitVertexShader() {
    return R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;

out vec3 Color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    Color = aColor;
    gl_Position = projection * view * vec4(aPos, 1.0);
}
)";
}

std::string Shader::GetUnlitFragmentShader() {
    return R"(
#version 330 core
out vec4 FragColor;

in vec3 Color;

void main() {
    FragColor = vec4(Color, 1.0);
}
)";
}

// ============================================
// Light Implementation
// ============================================

Light Light::Directional(const Vec3& dir, const Vec3& color, float intensity) {
    Light light;
    light.type = LightType::Directional;
    light.direction = dir.normalize();
    light.color = color;
    light.intensity = intensity;
    return light;
}

Light Light::Point(const Vec3& pos, const Vec3& color, float intensity, float range) {
    Light light;
    light.type = LightType::Point;
    light.position = pos;
    light.color = color;
    light.intensity = intensity;
    light.range = range;
    return light;
}

Light Light::Spot(const Vec3& pos, const Vec3& dir, const Vec3& color, float intensity, float range, float angle) {
    Light light;
    light.type = LightType::Spot;
    light.position = pos;
    light.direction = dir.normalize();
    light.color = color;
    light.intensity = intensity;
    light.range = range;
    light.spotAngle = angle;
    return light;
}

// ============================================
// Lighting System Implementation
// ============================================

void LightingSystem::AddLight(const Light& light) {
    if (lights.size() < 8) { // Max 8 lights for performance
        lights.push_back(light);
    }
}

void LightingSystem::RemoveLight(size_t index) {
    if (index < lights.size()) {
        lights.erase(lights.begin() + index);
    }
}

void LightingSystem::ClearLights() {
    lights.clear();
}

void LightingSystem::ApplyToShader(Shader& shader) {
    shader.SetVec3("ambientLight", ambientLight);
    
    // Apply each light
    for (size_t i = 0; i < lights.size(); ++i) {
        std::string prefix = "lights[" + std::to_string(i) + "]";
        const Light& currentLight = lights[i];
        
        shader.SetInt(prefix + ".type", static_cast<int>(currentLight.type));
        shader.SetVec3(prefix + ".position", currentLight.position);
        shader.SetVec3(prefix + ".direction", currentLight.direction);
        shader.SetVec3(prefix + ".color", currentLight.color);
        shader.SetFloat(prefix + ".intensity", currentLight.intensity);
        shader.SetFloat(prefix + ".range", currentLight.range);
        shader.SetFloat(prefix + ".spotAngle", currentLight.spotAngle);
    }
    
    shader.SetInt("numLights", static_cast<int>(lights.size()));
}

} // namespace vge