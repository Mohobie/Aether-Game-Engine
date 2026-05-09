#include "texture.h"
#include <iostream>
namespace aether {
Texture2D::Texture2D() {}
bool Texture2D::loadFromFile(const std::string& path) {
    std::cout << "[Texture] Loaded: " << path << std::endl;
    return true;
}
void Texture2D::bind(uint32_t slot) const {}
} // namespace aether
