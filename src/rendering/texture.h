#pragma once
#include <string>
#include <cstdint>

namespace vge {
class Texture {
    uint32_t textureId;
    int width, height, channels;
    bool loaded;
public:
    Texture();
    ~Texture();
    bool LoadFromFile(const std::string& path);
    bool LoadFromData(const unsigned char* data, int w, int h, int channels);
    void Bind(int slot = 0) const;
    void Unbind() const;
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    bool IsLoaded() const { return loaded; }
};
}