#pragma once

#include <string>

namespace VoxelEngine {
    class Shader {
    public:
        bool Load(const std::string& vertex, const std::string& fragment);
        void Use();
        void SetUniform(const std::string& name, float value);
    private:
        unsigned int program;
    };
}
