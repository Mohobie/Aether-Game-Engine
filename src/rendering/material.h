#pragma once
#include "texture.h"
#include "math/vec3.h"

namespace vge {
struct Material {
    Vec3 diffuseColor;
    Vec3 specularColor;
    float shininess;
    Texture* diffuseTexture;
    
    Material() : diffuseColor(1.0f, 1.0f, 1.0f), specularColor(1.0f, 1.0f, 1.0f), 
                 shininess(32.0f), diffuseTexture(nullptr) {}
};
}