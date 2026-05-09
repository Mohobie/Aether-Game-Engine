#pragma once
#include "vec3.h"
namespace aether {
class PerlinNoise {
public:
    PerlinNoise(unsigned int seed = 0);
    float noise(float x, float y, float z) const;
    float octaveNoise(float x, float y, float z, int octaves) const;
private:
    int p[512];
    float fade(float t) const;
    float lerp(float t, float a, float b) const;
    float grad(int hash, float x, float y, float z) const;
};
} // namespace aether
