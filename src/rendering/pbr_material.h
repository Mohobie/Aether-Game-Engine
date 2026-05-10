#pragma once
#include "math/vec3.h"
#include <string>

namespace vge {

// ============================================
// PBR Material Properties
// ============================================
struct PBRMaterial {
    // Base color (albedo)
    Vec3 albedo;
    float alpha;
    
    // Metallic-roughness workflow
    float metallic;
    float roughness;
    float ao; // Ambient occlusion
    
    // Emission for glowing materials
    Vec3 emission;
    float emissionStrength;
    
    // Clear coat for car paint, etc.
    float clearCoat;
    float clearCoatRoughness;
    
    // Subsurface scattering (skin, wax, etc.)
    float subsurface;
    Vec3 subsurfaceColor;
    
    // Index of refraction for transparent materials
    float ior;
    
    // Normal map strength
    float normalStrength;
    
    // Texture paths
    std::string albedoMap;
    std::string normalMap;
    std::string metallicMap;
    std::string roughnessMap;
    std::string aoMap;
    std::string emissionMap;
    std::string heightMap;
    
    PBRMaterial()
        : albedo(1.0f, 1.0f, 1.0f)
        , alpha(1.0f)
        , metallic(0.0f)
        , roughness(0.5f)
        , ao(1.0f)
        , emission(0, 0, 0)
        , emissionStrength(0.0f)
        , clearCoat(0.0f)
        , clearCoatRoughness(0.0f)
        , subsurface(0.0f)
        , subsurfaceColor(1.0f, 1.0f, 1.0f)
        , ior(1.5f)
        , normalStrength(1.0f)
    {}
    
    // Preset materials
    static PBRMaterial Gold();
    static PBRMaterial Silver();
    static PBRMaterial Copper();
    static PBRMaterial Iron();
    static PBRMaterial Plastic(Vec3 color);
    static PBRMaterial Rubber(Vec3 color);
    static PBRMaterial Wood();
    static PBRMaterial Stone();
    static PBRMaterial Glass();
    static PBRMaterial Water();
};

// ============================================
// PBR Shader
// ============================================
class PBRShader {
public:
    // Cook-Torrance BRDF helpers
    static float DistributionGGX(Vec3 N, Vec3 H, float roughness);
    static float GeometrySmith(Vec3 N, Vec3 V, Vec3 L, float roughness);
    static Vec3 FresnelSchlick(float cosTheta, Vec3 F0);
    static Vec3 FresnelSchlickRoughness(float cosTheta, Vec3 F0, float roughness);
    
    // Calculate PBR lighting
    static Vec3 CalculatePBR(
        const Vec3& albedo,
        float metallic,
        float roughness,
        float ao,
        const Vec3& normal,
        const Vec3& viewDir,
        const Vec3& lightDir,
        const Vec3& lightColor,
        float lightIntensity,
        const Vec3& F0
    );
    
    // Environment lighting (IBL)
    static Vec3 CalculateIBL(
        const Vec3& albedo,
        float metallic,
        float roughness,
        float ao,
        const Vec3& normal,
        const Vec3& viewDir,
        const Vec3& irradiance,
        const Vec3& prefilteredColor,
        float envBRDF
    );
};

// ============================================
// Environment Map for IBL
// ============================================
class EnvironmentMap {
private:
    uint32_t environmentTexture;
    uint32_t irradianceTexture;
    uint32_t prefilterTexture;
    uint32_t brdfLUTTexture;
    bool loaded;
    
public:
    EnvironmentMap();
    ~EnvironmentMap();
    
    bool LoadFromFile(const std::string& path);
    bool LoadFromHDR(const std::string& path);
    
    // Generate IBL maps from environment
    void GenerateIrradianceMap();
    void GeneratePrefilterMap();
    void GenerateBRDFLUT();
    
    // Bind for rendering
    void BindIrradiance(int slot);
    void BindPrefilter(int slot);
    void BindBRDFLUT(int slot);
    
    bool IsLoaded() const { return loaded; }
};

} // namespace vge