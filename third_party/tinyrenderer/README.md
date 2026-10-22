# TinyRenderer TGA utilities — C adaptation

Upstream: https://github.com/ssloy/tinyrenderer

These files are **modified C17 adaptations**, not unmodified upstream files:

- `tgaimage.h` / `tgaimage.c`: Plain image structs, explicit allocation/destruction,
  pixel access, flips, and uncompressed/RLE grayscale/RGB/RGBA TGA I/O.
- `starting_point.c`: The original three-pixel starting scene using the C API.
  It is a standalone reference example and is not part of `cpu_renderer`.

The original C++ `tgaimage.h`, `tgaimage.cpp`, and `starting_point.cpp`
(upstream `main.cpp`) came from the starting point linked in the upstream README:
https://github.com/ssloy/tinyrenderer/tree/706b2dfecff65daeb93de568ee2c2bd87f277860

`LICENSE.txt` was downloaded from upstream `master` on 2026-10-04;
the starting-point snapshot does not contain a license file. The original
license and attribution are retained unchanged.

C usage:

```c
TGAImage image = {0};
if(tgaImageInit(&image, 64, 64, TGA_RGB))
{
    tgaImageSet(&image, 7, 3, (TGAColor){{255, 255, 255, 255}});
    bool written = tgaImageWrite(&image, "framebuffer.tga", true, true);
    (void)written;
    tgaImageDestroy(&image);
}
```

Channels use BGRA order. Write origin and RLE options are explicit; `true, true`
preserves the original bottom-left origin and compression. Reading normalizes
pixels to top-left origin, as the upstream reader did. File headers are encoded
explicitly in little-endian bytes. Color-mapped images are not supported.
