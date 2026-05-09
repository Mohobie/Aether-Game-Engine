#pragma once
#include <string>
namespace aether {
class Shader {
public:
    bool load(const std::string& vertexSource, const std::string& fragmentSource);
    void bind();
    void setUniform(const std::string& name, float value);
private:
    unsigned int program = 0;
};
} // namespace aether
