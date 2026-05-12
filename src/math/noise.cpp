#include "noise.h"
#include <math>
namespace vge {
PerlinNoise::PerlinNoise(unsigned int seed) {
    for (int i = 0; i < 256; ++i) p[i] = i;
    for (int i = 0; i < 256; ++i) {
        int j = (seed + i) % 256;
        std::swap(p[i], p[j]);
    }
    for (int i = 0; i < 256; ++i) p[256 + i] = p[i];
}
float PerlinNoise::fade(float t) const { return t * t * t * (t * (t * 6 - 15) + 10); }
float PerlinNoise::lerp(float t, float a, float b) const { return a + t * (b - a); }
float PerlinNoise::grad(int hash, float x, float y, float z) const {
    int h = hash & 15;
    float u = h < 8 ? x : y;
    float v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}
float PerlinNoise::noise(float x, float y, float z) const {
    int X = (int)std::floor(x) & 255;
    int Y = (int)std::floor(y) & 255;
    int Z = (int)std::floor(z) & 255;
    x -= std::floor(x); y -= std::floor(y); z -= std::floor(z);
    float u = fade(x), v = fade(y), w = fade(z);
    int A = p[X] + Y, AA = p[A] + Z, AB = p[A + 1] + Z;
    int B = p[X + 1] + Y, BA = p[B] + Z, BB = p[B + 1] + Z;
    return lerp(w, lerp(v, lerp(u, grad(p[AA], x, y, z), grad(p[BA], x - 1, y, z)),
                           lerp(u, grad(p[AB], x, y - 1, z), grad(p[BB], x - 1, y - 1, z))),
                   lerp(v, lerp(u, grad(p[AA + 1], x, y, z - 1), grad(p[BA + 1], x - 1, y, z - 1)),
                           lerp(u, grad(p[AB + 1], x, y - 1, z - 1), grad(p[BB + 1], x - 1, y - 1, z - 1))));
}
float PerlinNoise::octaveNoise(float x, float y, float z, int octaves) const {
    float total = 0.0f, frequency = 1.0f, amplitude = 1.0f, maxValue = 0.0f;
    for (int i = 0; i < octaves; ++i) {
        total += noise(x * frequency, y * frequency, z * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= 0.5f;
        frequency *= 2.0f;
    }
    return total / maxValue;
}
} // namespace vge
