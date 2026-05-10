#include "rendering/framebuffer_renderer.h"
#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>

namespace vge {

FramebufferRenderer::FramebufferRenderer() 
    : fb_fd(-1), fb_mem(nullptr), width(0), height(0), 
      bpp(32), stride(0), initialized(false), back_buffer(nullptr) {}

FramebufferRenderer::~FramebufferRenderer() {
    if (initialized) Shutdown();
}

bool FramebufferRenderer::Initialize() {
    // Open framebuffer device
    fb_fd = open("/dev/fb0", O_RDWR);
    if (fb_fd < 0) {
        std::cerr << "[FB] Failed to open /dev/fb0 (need video group or root)" << std::endl;
        return false;
    }
    
    // Get screen info
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    
    if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo) < 0) {
        std::cerr << "[FB] Failed to get variable screen info" << std::endl;
        close(fb_fd);
        fb_fd = -1;
        return false;
    }
    
    if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo) < 0) {
        std::cerr << "[FB] Failed to get fixed screen info" << std::endl;
        close(fb_fd);
        fb_fd = -1;
        return false;
    }
    
    width = vinfo.xres;
    height = vinfo.yres;
    bpp = vinfo.bits_per_pixel;
    stride = finfo.line_length;
    
    std::cout << "[FB] Framebuffer: " << width << "x" << height 
              << " @ " << bpp << "bpp, stride=" << stride << std::endl;
    
    // Map framebuffer memory
    size_t screensize = height * stride;
    fb_mem = (uint8_t*)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
    if (fb_mem == MAP_FAILED) {
        std::cerr << "[FB] Failed to mmap framebuffer" << std::endl;
        close(fb_fd);
        fb_fd = -1;
        return false;
    }
    
    // Allocate back buffer
    back_buffer = new uint8_t[screensize];
    
    initialized = true;
    std::cout << "[FB] Framebuffer renderer initialized" << std::endl;
    return true;
}

void FramebufferRenderer::Shutdown() {
    if (!initialized) return;
    
    if (back_buffer) {
        delete[] back_buffer;
        back_buffer = nullptr;
    }
    
    if (fb_mem) {
        size_t screensize = height * stride;
        munmap(fb_mem, screensize);
        fb_mem = nullptr;
    }
    
    if (fb_fd >= 0) {
        close(fb_fd);
        fb_fd = -1;
    }
    
    initialized = false;
    std::cout << "[FB] Framebuffer renderer shutdown" << std::endl;
}

void FramebufferRenderer::Clear(uint32_t color) {
    if (!initialized || !back_buffer) return;
    
    size_t screensize = height * stride;
    
    if (bpp == 32) {
        uint32_t* buf = (uint32_t*)back_buffer;
        size_t pixels = screensize / 4;
        for (size_t i = 0; i < pixels; i++) {
            buf[i] = color;
        }
    } else if (bpp == 16) {
        uint16_t* buf = (uint16_t*)back_buffer;
        size_t pixels = screensize / 2;
        uint16_t c16 = ((color >> 8) & 0xF800) | ((color >> 5) & 0x07E0) | ((color >> 3) & 0x001F);
        for (size_t i = 0; i < pixels; i++) {
            buf[i] = c16;
        }
    }
}

void FramebufferRenderer::SetPixel(int x, int y, uint32_t color) {
    if (!initialized || !back_buffer) return;
    if (x < 0 || x >= width || y < 0 || y >= height) return;
    
    size_t offset = y * stride + x * (bpp / 8);
    
    if (bpp == 32) {
        *(uint32_t*)(back_buffer + offset) = color;
    } else if (bpp == 16) {
        uint16_t c16 = ((color >> 8) & 0xF800) | ((color >> 5) & 0x07E0) | ((color >> 3) & 0x001F);
        *(uint16_t*)(back_buffer + offset) = c16;
    }
}

void FramebufferRenderer::DrawRect(int x, int y, int w, int h, uint32_t color) {
    if (!initialized || !back_buffer) return;
    
    for (int dy = 0; dy < h; dy++) {
        for (int dx = 0; dx < w; dx++) {
            SetPixel(x + dx, y + dy, color);
        }
    }
}

void FramebufferRenderer::DrawLine(int x0, int y0, int x1, int y1, uint32_t color) {
    if (!initialized || !back_buffer) return;
    
    // Bresenham's line algorithm
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    
    while (true) {
        SetPixel(x0, y0, color);
        
        if (x0 == x1 && y0 == y1) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void FramebufferRenderer::SwapBuffers() {
    if (!initialized || !back_buffer || !fb_mem) return;
    
    size_t screensize = height * stride;
    memcpy(fb_mem, back_buffer, screensize);
}

} // namespace vge