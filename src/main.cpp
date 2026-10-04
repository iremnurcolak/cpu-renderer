#include <cstdint>
#include <iostream>

struct Color
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

constexpr int framebufferWidth = 800;
constexpr int framebufferHeight = 600;

// Pixels are stored row by row, starting at the top-left corner.
Color framebuffer[framebufferWidth * framebufferHeight]{};

void setPixel(int x, int y, Color color)
{
    if (x < 0 || x >= framebufferWidth || y < 0 || y >= framebufferHeight)
    {
        return;
    }

    framebuffer[y * framebufferWidth + x] = color;
}

int main()
{
    std::cout << "CPU Renderer\n";
    return 0;
}
