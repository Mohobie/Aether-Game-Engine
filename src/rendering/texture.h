#pragma once
#include <string>
#include <glad/gl.h>

namespace vge {
class Texture {
    uint32_t textureId;
    int width, height;
public:
    Texture();
    ~Texture();
    bool LoadFromFile(const std::string& path);
    void Bind(uint32_t slot = 0);
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
};
}