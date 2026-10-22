/* C17 adaptation of TinyRenderer's TGA utilities; see LICENSE.txt. */
#ifndef TGAIMAGE_H
#define TGAIMAGE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum TGAFormat { TGA_GRAYSCALE = 1, TGA_RGB = 3, TGA_RGBA = 4 } TGAFormat;

/** @brief Pixel channels in BGRA order; grayscale uses bgra[0]. */
typedef struct TGAColor {
    uint8_t bgra[4];
} TGAColor;

/** @brief Owned image storage. Initialize to zero before using the image API. */
typedef struct TGAImage {
    int width;
    int height;
    int bytesPerPixel;
    uint8_t *data;
} TGAImage;

/**
 * @brief Allocate zeroed pixels, replacing any storage already owned by the image.
 * @param[in,out] image Image to initialize; use {0} before the first call.
 * @param[in] width Positive image width, at most 65535.
 * @param[in] height Positive image height, at most 65535.
 * @param[in] format Number of bytes per pixel: grayscale, RGB, or RGBA.
 * @return true on success; false for invalid dimensions or allocation failure.
 */
bool tgaImageInit(TGAImage *image, int width, int height, TGAFormat format);

/**
 * @brief Release image pixels and reset the image to zero; repeated calls are safe.
 * @param[in,out] image Image to destroy, or NULL.
 */
void tgaImageDestroy(TGAImage *image);

/**
 * @brief Read an uncompressed or RLE grayscale/RGB/RGBA TGA into an owned image.
 * @param[in,out] image Zero-initialized or previously initialized destination image.
 * @param[in] filename Input filename.
 * @return true on success; false for invalid, truncated, unsupported, or unreadable files.
 * @note Pixels are normalized to top-left origin, matching the upstream reader.
 * @note On failure the previous image is retained.
 */
bool tgaImageRead(TGAImage *image, const char *filename);

/**
 * @brief Write a TGA image with an explicit origin and optional RLE compression.
 * @param[in] image Initialized source image.
 * @param[in] filename Output filename.
 * @param[in] vflip true labels stored rows as bottom-left origin; false uses top-left.
 * @param[in] rle true writes RLE packets; false writes uncompressed pixels.
 * @return true on success; false for invalid images or file I/O failure.
 */
bool tgaImageWrite(const TGAImage *image, const char *filename, bool vflip, bool rle);

/**
 * @brief Read a pixel; out-of-bounds coordinates return a zero color.
 * @param[in] image Source image.
 * @param[in] x Pixel column.
 * @param[in] y Pixel row in memory.
 * @return Pixel channels, with unused channels set to zero.
 */
TGAColor tgaImageGet(const TGAImage *image, int x, int y);

/**
 * @brief Write pixel channels, ignoring out-of-bounds coordinates.
 * @param[in,out] image Destination image.
 * @param[in] x Pixel column.
 * @param[in] y Pixel row in memory.
 * @param[in] color BGRA color; grayscale writes only its first channel.
 */
void tgaImageSet(TGAImage *image, int x, int y, TGAColor color);

/**
 * @brief Mirror an image horizontally in place.
 * @param[in,out] image Initialized image.
 */
void tgaImageFlipHorizontally(TGAImage *image);

/**
 * @brief Mirror an image vertically in place.
 * @param[in,out] image Initialized image.
 */
void tgaImageFlipVertically(TGAImage *image);

#endif
