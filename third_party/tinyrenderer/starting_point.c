/* C17 adaptation of the upstream example; see README.md and LICENSE.txt. */
#include "tgaimage.h"

/**
 * @brief Save the three original starting-point vertices as a TGA image.
 * @return 0 on success or 1 on allocation or output failure.
 */
int main(void)
{
    const TGAColor white = {{255, 255, 255, 255}};
    TGAImage framebuffer = {0};
    if(!tgaImageInit(&framebuffer, 64, 64, TGA_RGB))
        return 1;
    tgaImageSet(&framebuffer, 7, 3, white);
    tgaImageSet(&framebuffer, 12, 37, white);
    tgaImageSet(&framebuffer, 62, 53, white);
    const bool success = tgaImageWrite(&framebuffer, "framebuffer.tga", true, true);
    tgaImageDestroy(&framebuffer);
    return success ? 0 : 1;
}
