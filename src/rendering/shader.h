#pragma once
#include <string>
#include <glad/gl.h>
#include "math/vec3.h"
#include "math/mat4.h"

namespace vge {
class Shader {
    uint32_t programId;
public:
    Shader();
    ~Shader();
    bool LoadFromFile(const std::string& vertexPath, const std::string& fragmentPath);
    void Use();
    void SetUniform(const std::string& name, const Mat4& value);
};
}