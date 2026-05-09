#include "rendering/texture.h"
#include <iostream>

// Stub implementation when OpenGL is not available
namespace vge {

Texture::Texture() : textureId(0), width(0), height(0), channels(0), loaded(false) {}

Texture::~Texture() {
    // Would delete OpenGL texture
}

bool Texture::LoadFromFile(const std::string& path) {
    std::cout << "[Texture] Stub - would load " << path << std::endl;
    loaded = true;
    return true;
}

bool Texture::LoadFromData(const unsigned char* data, int w, int h, int channels) {
    width = w;
    height = h;
    this->channels = channels;
    loaded = true;
    return true;
}

void Texture::Bind(int slot) const {
    // Would bind OpenGL texture
}

void Texture::Unbind() const {
    // Would unbind texture
}

} // namespace vge