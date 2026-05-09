#include "texture.h"
#include <iostream>

namespace vge {
Texture::Texture() : textureId(0), width(0), height(0) {}

Texture::~Texture() {
    if (textureId) glDeleteTextures(1, &textureId);
}

bool Texture::LoadFromFile(const std::string& path) {
    // Stub: Would use stb_image to load PNG/JPG
    std::cout << "[Texture] Loading: " << path << " (stub)" << std::endl;
    
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    // Placeholder white texture
    unsigned char white[] = {255, 255, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    width = 1;
    height = 1;
    return true;
}

void Texture::Bind(uint32_t slot) {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, textureId);
}
}