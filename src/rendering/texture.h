#pragma once
#include <string>

namespace VoxelEngine {
    class Texture {
    public:
        bool Load(const std::string& path);
        void Bind(unsigned int slot);
        void Unbind();
        int GetWidth() const;
        int GetHeight() const;
        unsigned int GetID() const;
    private:
        unsigned int id = 0;
        int width = 0;
        int height = 0;
    };
}
