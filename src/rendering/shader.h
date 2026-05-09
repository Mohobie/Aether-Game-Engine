#pragma once
#include <string>
#include <cstdint>
#include "math/vec3.h"
#include "math/mat4.h"

namespace vge {
class Shader {
    uint32_t programId;
    bool initialized;
public:
    Shader();
    ~Shader();
    bool LoadFromFile(const std::string& vertexPath, const std::string& fragmentPath);
    bool LoadFromSource(const std::string& vertexSource, const std::string& fragmentSource);
    void Bind() const;
    void SetInt(const std::string& name, int value) const;
    void SetFloat(const std::string& name, float value) const;
    void SetVec3(const std::string& name, const Vec3& value) const;
    void SetMat4(const std::string& name, const Mat4& value) const;
};
}