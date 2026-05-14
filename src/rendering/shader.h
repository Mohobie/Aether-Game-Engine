#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "math/vec3.h"

namespace vge {

// Shader program
class Shader {
private:
    uint32_t programID;
    std::unordered_map<std::string, int> uniformLocations;
    bool loaded;
    
public:
    Shader();
    ~Shader();
    
    // Load from source strings
    bool LoadFromSource(const std::string& vertexSource, const std::string& fragmentSource);
    
    // Load from files
    bool LoadFromFile(const std::string& vertexPath, const std::string& fragmentPath);
    
    // Use this shader
    void Bind() const;
    void Unbind() const;
    
    // Set uniforms
    void SetFloat(const std::string& name, float value);
    void SetInt(const std::string& name, int value);
    void SetBool(const std::string& name, bool value);
    void SetVec2(const std::string& name, float x, float y);
    void SetVec3(const std::string& name, const Vec3& value);
    void SetVec4(const std::string& name, float x, float y, float z, float w);
    void SetMat4(const std::string& name, const float* matrix);
    
    bool IsLoaded() const { return loaded; }
    uint32_t GetID() const { return programID; }
    
    // Default shaders
    static std::string GetDefaultVertexShader();
    static std::string GetDefaultFragmentShader();
    static std::string GetUnlitVertexShader();
    static std::string GetUnlitFragmentShader();
};

// ============================================
// Lighting System
// ============================================

enum class LightType {
    Directional,
    Point,
    Spot
};

struct Light {
    LightType type;
    Vec3 position;
    Vec3 direction;
    Vec3 color;
    float intensity;
    float range;
    float spotAngle;
    
    Light() 
        : type(LightType::Point)
        , position(0, 0, 0)
        , direction(0, -1, 0)
        , color(1, 1, 1)
        , intensity(1.0f)
        , range(10.0f)
        , spotAngle(45.0f)
    {}
    
    // Factory methods
    static Light Directional(const Vec3& dir, const Vec3& color, float intensity);
    static Light Point(const Vec3& pos, const Vec3& color, float intensity, float range);
    static Light Spot(const Vec3& pos, const Vec3& dir, const Vec3& color, float intensity, float range, float angle);
};

// Lighting manager
class LightingSystem {
private:
    std::vector<Light> lights;
    Vec3 ambientLight;
    
public:
    LightingSystem() : ambientLight(0.2f, 0.2f, 0.2f) {}
    
    // Add lights
    void AddLight(const Light& light);
    void RemoveLight(size_t index);
    void ClearLights();
    
    // Get lights
    const std::vector<Light>& GetLights() const { return lights; }
    size_t GetLightCount() const { return lights.size(); }
    
    // Ambient
    void SetAmbient(const Vec3& color) { ambientLight = color; }
    Vec3 GetAmbient() const { return ambientLight; }
    
    // Apply to shader
    void ApplyToShader(Shader& shader);
};

} // namespace vge