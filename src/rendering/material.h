#pragma once
#include "texture.h"
#include "shader.h"
#include <memory>

namespace VoxelEngine {
    struct Material {
        std::shared_ptr<Shader> shader;
        std::shared_ptr<Texture> diffuse;
        std::shared_ptr<Texture> normal;
        float shininess = 32.0f;
    };
}
