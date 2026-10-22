/* C17 adaptation of TinyRenderer's TGA utilities; see LICENSE.txt. */
#include "tgaimage.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Allocate zeroed pixels, replacing any storage already owned by the image.
 * @param[in,out] image Image to initialize; use {0} before the first call.
 * @param[in] width Positive image width, at most 65535.
 * @param[in] height Positive image height, at most 65535.
 * @param[in] format Number of bytes per pixel: grayscale, RGB, or RGBA.
 * @return true on success; false for invalid dimensions or allocation failure.
 */
bool tgaImageInit(TGAImage *image, int width, int height, TGAFormat format)
{
    if(!image || width <= 0 || height <= 0 || width > 65535 || height > 65535
        || (format != TGA_GRAYSCALE && format != TGA_RGB && format != TGA_RGBA))
        return false;
    if((size_t)width > SIZE_MAX / (size_t)height / (size_t)format)
        return false;
    uint8_t *data = calloc((size_t)width * (size_t)height, (size_t)format);
    if(!data)
        return false;
    free(image->data);
    *image = (TGAImage){width, height, format, data};
    return true;
}

/**
 * @brief Release image pixels and reset the image to zero; repeated calls are safe.
 * @param[in,out] image Image to destroy, or NULL.
 */
void tgaImageDestroy(TGAImage *image)
{
    if(image)
    {
        free(image->data);
        *image = (TGAImage){0};
    }
}

/**
 * @brief Decode bounded RLE packets from an open TGA stream.
 * @param[in] file Stream positioned at the first pixel packet.
 * @param[in,out] image Allocated destination image.
 * @return true if every pixel was read without packet overflow or truncation.
 */
static bool readRle(FILE *file, TGAImage *image)
{
    const size_t count = (size_t)image->width * (size_t)image->height;
    const size_t bpp = (size_t)image->bytesPerPixel;
    size_t current = 0;
    while(current < count)
    {
        const int packet = fgetc(file);
        if(packet == EOF)
            return false;
        const size_t length = (size_t)(packet & 127) + 1;
        if(length > count - current)
            return false;
        if(packet & 128)
        {
            uint8_t pixel[4] = {0};
            if(fread(pixel, 1, bpp, file) != bpp)
                return false;
            for(size_t i = 0; i < length; ++i)
                memcpy(image->data + (current + i) * bpp, pixel, bpp);
        }
        else if(fread(image->data + current * bpp, bpp, length, file) != length)
            return false;
        current += length;
    }
    return true;
}

/**
 * @brief Read an uncompressed or RLE grayscale/RGB/RGBA TGA into an owned image.
 * @param[in,out] image Zero-initialized or previously initialized destination image.
 * @param[in] filename Input filename.
 * @return true on success; false for invalid, truncated, unsupported, or unreadable files.
 * @note Pixels are normalized to top-left origin, matching the upstream reader.
 * @note On failure the previous image is retained.
 */
bool tgaImageRead(TGAImage *image, const char *filename)
{
    if(!image || !filename)
        return false;
    FILE *file = fopen(filename, "rb");
    if(!file)
        return false;
    uint8_t header[18];
    TGAImage loaded = {0};
    bool success = false;
    if(fread(header, 1, sizeof(header), file) != sizeof(header))
        goto done;
    const int width = header[12] | ((int)header[13] << 8);
    const int height = header[14] | ((int)header[15] << 8);
    const int type = header[2];
    const int bpp = header[16] / 8;
    if(header[1] != 0 || (header[16] != 8 && header[16] != 24 && header[16] != 32)
        || !((bpp == 1 && (type == 3 || type == 11))
            || (bpp >= 3 && (type == 2 || type == 10))))
        goto done;
    if(!tgaImageInit(&loaded, width, height, (TGAFormat)bpp)
        || fseek(file, header[0], SEEK_CUR) != 0)
        goto done;
    if(type == 10 || type == 11)
    {
        if(!readRle(file, &loaded))
            goto done;
    }
    else
    {
        const size_t size = (size_t)width * (size_t)height * (size_t)bpp;
        if(fread(loaded.data, 1, size, file) != size)
            goto done;
    }
    if(!(header[17] & 0x20))
        tgaImageFlipVertically(&loaded);
    if(header[17] & 0x10)
        tgaImageFlipHorizontally(&loaded);
    tgaImageDestroy(image);
    *image = loaded;
    loaded = (TGAImage){0};
    success = true;
done:
    tgaImageDestroy(&loaded);
    fclose(file);
    return success;
}

/**
 * @brief Encode image pixels into raw and repeated TGA packets of at most 128 pixels.
 * @param[in] file Open destination stream.
 * @param[in] image Initialized source image.
 * @return true unless writing a packet fails.
 */
static bool writeRle(FILE *file, const TGAImage *image)
{
    const size_t count = (size_t)image->width * (size_t)image->height;
    const size_t bpp = (size_t)image->bytesPerPixel;
    size_t current = 0;
    while(current < count)
    {
        const size_t start = current * bpp;
        size_t length = 1;
        bool raw = true;
        while(current + length < count && length < 128)
        {
            const bool equal = memcmp(image->data + (current + length - 1) * bpp,
                image->data + (current + length) * bpp, bpp) == 0;
            if(length == 1)
                raw = !equal;
            if(raw && equal)
            {
                --length;
                break;
            }
            if(!raw && !equal)
                break;
            ++length;
        }
        const size_t bytes = raw ? length * bpp : bpp;
        if(fputc((int)(raw ? length - 1 : length + 127), file) == EOF
            || fwrite(image->data + start, 1, bytes, file) != bytes)
            return false;
        current += length;
    }
    return true;
}

/**
 * @brief Write a TGA image with an explicit origin and optional RLE compression.
 * @param[in] image Initialized source image.
 * @param[in] filename Output filename.
 * @param[in] vflip true labels stored rows as bottom-left origin; false uses top-left.
 * @param[in] rle true writes RLE packets; false writes uncompressed pixels.
 * @return true on success; false for invalid images or file I/O failure.
 */
bool tgaImageWrite(const TGAImage *image, const char *filename, bool vflip, bool rle)
{
    if(!image || !image->data || !filename)
        return false;
    FILE *file = fopen(filename, "wb");
    if(!file)
        return false;
    uint8_t header[18] = {0};
    header[2] = (uint8_t)(image->bytesPerPixel == 1 ? (rle ? 11 : 3) : (rle ? 10 : 2));
    header[12] = (uint8_t)image->width;
    header[13] = (uint8_t)(image->width >> 8);
    header[14] = (uint8_t)image->height;
    header[15] = (uint8_t)(image->height >> 8);
    header[16] = (uint8_t)(image->bytesPerPixel * 8);
    header[17] = (uint8_t)((vflip ? 0 : 0x20) | (image->bytesPerPixel == 4 ? 8 : 0));
    const uint8_t references[8] = {0};
    const char footer[18] = "TRUEVISION-XFILE.";
    bool success = fwrite(header, 1, sizeof(header), file) == sizeof(header);
    if(success)
    {
        const size_t size = (size_t)image->width * (size_t)image->height * (size_t)image->bytesPerPixel;
        success = rle ? writeRle(file, image) : fwrite(image->data, 1, size, file) == size;
    }
    if(success)
        success = fwrite(references, 1, sizeof(references), file) == sizeof(references)
            && fwrite(footer, 1, sizeof(footer), file) == sizeof(footer);
    if(fclose(file) != 0)
        success = false;
    return success;
}

/**
 * @brief Read a pixel; out-of-bounds coordinates return a zero color.
 * @param[in] image Source image.
 * @param[in] x Pixel column.
 * @param[in] y Pixel row in memory.
 * @return Pixel channels, with unused channels set to zero.
 */
TGAColor tgaImageGet(const TGAImage *image, int x, int y)
{
    TGAColor color = {{0}};
    if(image && image->data && x >= 0 && y >= 0 && x < image->width && y < image->height)
        memcpy(color.bgra, image->data + ((size_t)y * (size_t)image->width + (size_t)x)
            * (size_t)image->bytesPerPixel, (size_t)image->bytesPerPixel);
    return color;
}

/**
 * @brief Write pixel channels, ignoring out-of-bounds coordinates.
 * @param[in,out] image Destination image.
 * @param[in] x Pixel column.
 * @param[in] y Pixel row in memory.
 * @param[in] color BGRA color; grayscale writes only its first channel.
 */
void tgaImageSet(TGAImage *image, int x, int y, TGAColor color)
{
    if(image && image->data && x >= 0 && y >= 0 && x < image->width && y < image->height)
        memcpy(image->data + ((size_t)y * (size_t)image->width + (size_t)x)
            * (size_t)image->bytesPerPixel, color.bgra, (size_t)image->bytesPerPixel);
}

/**
 * @brief Mirror an image horizontally in place.
 * @param[in,out] image Initialized image.
 */
void tgaImageFlipHorizontally(TGAImage *image)
{
    if(!image || !image->data)
        return;
    for(int y = 0; y < image->height; ++y)
        for(int x = 0; x < image->width / 2; ++x)
        {
            const TGAColor left = tgaImageGet(image, x, y);
            const TGAColor right = tgaImageGet(image, image->width - 1 - x, y);
            tgaImageSet(image, x, y, right);
            tgaImageSet(image, image->width - 1 - x, y, left);
        }
}

/**
 * @brief Mirror an image vertically in place.
 * @param[in,out] image Initialized image.
 */
void tgaImageFlipVertically(TGAImage *image)
{
    if(!image || !image->data)
        return;
    for(int y = 0; y < image->height / 2; ++y)
        for(int x = 0; x < image->width; ++x)
        {
            const TGAColor top = tgaImageGet(image, x, y);
            const TGAColor bottom = tgaImageGet(image, x, image->height - 1 - y);
            tgaImageSet(image, x, y, bottom);
            tgaImageSet(image, x, image->height - 1 - y, top);
        }
}
