#pragma once
#include <cstdint>
#include <string>

namespace vge {

// Linux framebuffer renderer - no X11 or OpenGL needed!
// Direct pixel access to /dev/fb0
class FramebufferRenderer {
private:
    int fb_fd;
    uint8_t* fb_mem;
    int width;
    int height;
    int bpp;
    int stride;
    bool initialized;
    
    // Double buffering
    uint8_t* back_buffer;
    
public:
    FramebufferRenderer();
    ~FramebufferRenderer();
    
    bool Initialize();
    void Shutdown();
    
    void Clear(uint32_t color);
    void SetPixel(int x, int y, uint32_t color);
    void DrawRect(int x, int y, int w, int h, uint32_t color);
    void DrawLine(int x0, int y0, int x1, int y1, uint32_t color);
    
    void SwapBuffers();
    
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    bool IsInitialized() const { return initialized; }
};

// Color helpers
inline uint32_t RGB(uint8_t r, uint8_t g, uint8_t b) {
    return (r << 16) | (g << 8) | b;
}

inline uint32_t RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return (a << 24) | (r << 16) | (g << 8) | b;
}

} // namespace vge