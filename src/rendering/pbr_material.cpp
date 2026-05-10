#include "pbr_material.h"
#include <cmath>
#include <iostream>

namespace vge {

// ============================================
// PBR Material Presets
// ============================================

PBRMaterial PBRMaterial::Gold() {
    PBRMaterial mat;
    mat.albedo = Vec3(1.0f, 0.78f, 0.34f);
    mat.metallic = 1.0f;
    mat.roughness = 0.15f;
    return mat;
}

PBRMaterial PBRMaterial::Silver() {
    PBRMaterial mat;
    mat.albedo = Vec3(0.97f, 0.96f, 0.91f);
    mat.metallic = 1.0f;
    mat.roughness = 0.2f;
    return mat;
}

PBRMaterial PBRMaterial::Copper() {
    PBRMaterial mat;
    mat.albedo = Vec3(0.95f, 0.64f, 0.54f);
    mat.metallic = 1.0f;
    mat.roughness = 0.25f;
    return mat;
}

PBRMaterial PBRMaterial::Iron() {
    PBRMaterial mat;
    mat.albedo = Vec3(0.77f, 0.78f, 0.78f);
    mat.metallic = 1.0f;
    mat.roughness = 0.4f;
    return mat;
}

PBRMaterial PBRMaterial::Plastic(Vec3 color) {
    PBRMaterial mat;
    mat.albedo = color;
    mat.metallic = 0.0f;
    mat.roughness = 0.3f;
    return mat;
}

PBRMaterial PBRMaterial::Rubber(Vec3 color) {
    PBRMaterial mat;
    mat.albedo = color;
    mat.metallic = 0.0f;
    mat.roughness = 0.9f;
    return mat;
}

PBRMaterial PBRMaterial::Wood() {
    PBRMaterial mat;
    mat.albedo = Vec3(0.6f, 0.4f, 0.2f);
    mat.metallic = 0.0f;
    mat.roughness = 0.6f;
    return mat;
}

PBRMaterial PBRMaterial::Stone() {
    PBRMaterial mat;
    mat.albedo = Vec3(0.5f, 0.5f, 0.5f);
    mat.metallic = 0.0f;
    mat.roughness = 0.8f;
    return mat;
}

PBRMaterial PBRMaterial::Glass() {
    PBRMaterial mat;
    mat.albedo = Vec3(1.0f, 1.0f, 1.0f);
    mat.metallic = 0.0f;
    mat.roughness = 0.05f;
    mat.alpha = 0.1f;
    mat.ior = 1.45f;
    return mat;
}

PBRMaterial PBRMaterial::Water() {
    PBRMaterial mat;
    mat.albedo = Vec3(0.0f, 0.3f, 0.6f);
    mat.metallic = 0.0f;
    mat.roughness = 0.1f;
    mat.alpha = 0.7f;
    mat.ior = 1.33f;
    return mat;
}

// ============================================
// PBR Shader Implementation
// ============================================

float PBRShader::DistributionGGX(Vec3 N, Vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    
    float NdotH = std::max(N.dot(H), 0.0f);
    float NdotH2 = NdotH * NdotH;
    
    float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom = 3.14159f * denom * denom;
    
    return a2 / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0f);
    float k = (r * r) / 8.0f;
    
    float denom = NdotV * (1.0f - k) + k;
    
    return NdotV / denom;
}

float PBRShader::GeometrySmith(Vec3 N, Vec3 V, Vec3 L, float roughness) {
    float NdotV = std::max(N.dot(V), 0.0f);
    float NdotL = std::max(N.dot(L), 0.0f);
    
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

Vec3 PBRShader::FresnelSchlick(float cosTheta, Vec3 F0) {
    return F0 + (Vec3(1.0f, 1.0f, 1.0f) - F0) * std::pow(1.0f - cosTheta, 5.0f);
}

Vec3 PBRShader::FresnelSchlickRoughness(float cosTheta, Vec3 F0, float roughness) {
    return F0 + (Vec3(std::max(1.0f - roughness, F0.x), 
                      std::max(1.0f - roughness, F0.y),
                      std::max(1.0f - roughness, F0.z)) - F0) * 
           std::pow(1.0f - cosTheta, 5.0f);
}

Vec3 PBRShader::CalculatePBR(
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
) {
    Vec3 N = normal.normalize();
    Vec3 V = viewDir.normalize();
    Vec3 L = lightDir.normalize();
    Vec3 H = (V + L).normalize();
    
    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    Vec3 F = FresnelSchlick(std::max(H.dot(V), 0.0f), F0);
    
    Vec3 kS = F;
    Vec3 kD = Vec3(1.0f, 1.0f, 1.0f) - kS;
    kD = kD * (1.0f - metallic);
    
    Vec3 numerator = Vec3(NDF * G, NDF * G, NDF * G) * F;
    float denominator = 4.0f * std::max(N.dot(V), 0.0f) * std::max(N.dot(L), 0.0f) + 0.001f;
    Vec3 specular = numerator / denominator;
    
    // Lambertian diffuse
    float NdotL = std::max(N.dot(L), 0.0f);
    Vec3 diffuse = kD * (albedo / 3.14159f);
    
    // Combine
    Vec3 Lo = (diffuse + specular) * (lightColor * lightIntensity * NdotL);
    
    // Ambient occlusion
    Lo = Lo * ao;
    
    return Lo;
}

Vec3 PBRShader::CalculateIBL(
    const Vec3& albedo,
    float metallic,
    float roughness,
    float ao,
    const Vec3& normal,
    const Vec3& viewDir,
    const Vec3& irradiance,
    const Vec3& prefilteredColor,
    float envBRDF
) {
    Vec3 N = normal.normalize();
    Vec3 V = viewDir.normalize();
    
    // Calculate F0
    Vec3 F0 = Vec3(0.04f, 0.04f, 0.04f);
    // Simple lerp
    F0 = F0 + (albedo - F0) * metallic;
    
    // Fresnel
    Vec3 F = FresnelSchlickRoughness(std::max(N.dot(V), 0.0f), F0, roughness);
    
    Vec3 kS = F;
    Vec3 kD = Vec3(1.0f, 1.0f, 1.0f) - kS;
    kD = kD * (1.0f - metallic);
    
    // Diffuse IBL
    Vec3 diffuse = irradiance * albedo;
    
    // Specular IBL
    Vec3 specular = prefilteredColor * (F * envBRDF);
    
    // Combine
    Vec3 ambient = (kD * diffuse + specular) * ao;
    
    return ambient;
}

// ============================================
// Environment Map Implementation
// ============================================

EnvironmentMap::EnvironmentMap() 
    : environmentTexture(0)
    , irradianceTexture(0)
    , prefilterTexture(0)
    , brdfLUTTexture(0)
    , loaded(false) {
}

EnvironmentMap::~EnvironmentMap() {
    // Would delete OpenGL textures
}

bool EnvironmentMap::LoadFromFile(const std::string& path) {
    std::cout << "[EnvironmentMap] Loading: " << path << std::endl;
    // Would load cubemap texture
    loaded = true;
    return true;
}

bool EnvironmentMap::LoadFromHDR(const std::string& path) {
    std::cout << "[EnvironmentMap] Loading HDR: " << path << std::endl;
    // Would load HDR environment map and convert to cubemap
    loaded = true;
    return true;
}

void EnvironmentMap::GenerateIrradianceMap() {
    std::cout << "[EnvironmentMap] Generating irradiance map" << std::endl;
    // Would convolve environment map for diffuse IBL
}

void EnvironmentMap::GeneratePrefilterMap() {
    std::cout << "[EnvironmentMap] Generating prefilter map" << std::endl;
    // Would prefilter for specular IBL
}

void EnvironmentMap::GenerateBRDFLUT() {
    std::cout << "[EnvironmentMap] Generating BRDF LUT" << std::endl;
    // Would generate 2D BRDF lookup texture
}

void EnvironmentMap::BindIrradiance(int slot) {
    // Would bind irradiance texture
}

void EnvironmentMap::BindPrefilter(int slot) {
    // Would bind prefilter texture
}

void EnvironmentMap::BindBRDFLUT(int slot) {
    // Would bind BRDF LUT texture
}

} // namespace vge
