# CPU Renderer

A C17 renderer project for learning pixel, line, and triangle drawing on the CPU.
Image output uses a C adaptation of TinyRenderer's TGA utilities. No C++ compiler
or C++ runtime is required.

The project was migrated from C++ to C so learning effort could stay focused on
rasterization rather than learning an unfamiliar language alongside it.

## Building and running

Requirements: CMake 3.20 or later and a compiler with C17 support.

From the project root:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/cpu_renderer
```

The executable writes `framebuffer.tga` to its working directory. Without
arguments it draws three RGB-interpolated triangles on a 128 × 128 framebuffer.
Generated images and build directories are ignored by Git.

The existing `build/` directory can be reconfigured with these commands after
the C migration. Reconfigure `build-release/` before using it as well.

On Windows with a multi-configuration generator:

```powershell
cmake -S . -B build
cmake --build build --config Debug
.\build\Debug\cpu_renderer.exe
```

## C API and image ownership

Images are plain structs. Initialize them to zero, allocate pixels explicitly,
and release the pixels when finished. Pass image pointers to drawing functions.
`TGAColor` stores four channels in **BGRA** order.

```c
#include "colors.h"
#include "triangle_renderer.h"

int main(void)
{
    TGAImage framebuffer = {0};
    if(!tgaImageInit(&framebuffer, 128, 128, TGA_RGB))
        return 1;

    // Counterclockwise vertices, followed by their respective colors.
    drawTriangleWithBoundingBox(7, 45, 45, 60, 35, 100,
        red, green, blue, &framebuffer);

    const bool success = tgaImageWrite(&framebuffer, "framebuffer.tga", true, true);
    tgaImageDestroy(&framebuffer);
    return success ? 0 : 1;
}
```

- `tgaImageInit` allocates black grayscale, RGB, or RGBA pixels.
- `tgaImageSet(&image, x, y, color)` writes a pixel and ignores out-of-bounds coordinates.
- `tgaImageGet(&image, x, y)` returns a pixel value.
- `tgaImageRead` supports uncompressed and RLE grayscale/RGB/RGBA TGA files.
- `tgaImageWrite` takes explicit origin and compression arguments. `true, true`
  preserves the original bottom-left origin and RLE output.
- `tgaImageDestroy` frees owned storage. An initialized image must not be shallow
  copied and then destroyed twice; use pointers when sharing it.

A grayscale sample can be written with a compound literal:

```c
uint8_t z = 128;
tgaImageSet(&zbuffer, x, y, (TGAColor){{z, 0, 0, 0}});
```

Initialize `zbuffer` with `TGA_GRAYSCALE` to store only the first channel.

## Line and triangle rendering

- `drawLineBarycentric` samples a segment at parameter increments of `0.02`.
  Long lines can have gaps.
- `drawLineInterpolated` uses integer error accumulation, supports all directions,
  and includes both endpoints. Coincident endpoints draw one pixel.
- `drawTriangleWithMyScanlineV1` computes floating-point scanline intersections.
  It requires three distinct vertex y coordinates; horizontal-edge cases are skipped.
- `drawTriangleWithMyScanlineV2` computes integer scanline intersections and excludes
  the right endpoint of each horizontal span.
- `drawTriangleWithBoundingBox` interpolates the B, G, and R channels of three
  vertex colors; output alpha is 255.
- `drawTriangleWithBoundingBoxDepth` additionally accepts three mapped depth values
  and an optional grayscale depth image pointer. This replaces the C++ overload.

The bounding-box paths skip triangles with signed area below 1 pixel squared,
including clockwise and degenerate triangles when y increases upward. Assign the
same color to all three vertices to draw a uniformly colored face.

```c
// az, bz, and cz are mapped to [0, 255] before this call.
drawTriangleWithBoundingBoxDepth(ax, ay, bx, by, cx, cy,
    az, bz, cz, red, green, blue, &framebuffer, &zbuffer);
```

All project functions have English Doxygen comments in the headers and sources.

## Rendering OBJ models

Place models under `obj/` and run from the project root:

```bash
./build/cpu_renderer diablo3_pose.obj
./build/cpu_renderer --wireframe diablo3_pose.obj
./build/cpu_renderer --filled obj/african_head/african_head.obj
./build/cpu_renderer obj/boggie/head.obj --filled
```

The mode flag can precede or follow the model path; filled rendering is the default.
Modes cannot be combined with each other or with `--benchmark`.

| Mode | Color output | Depth output |
|---|---|---|
| `--wireframe` | `obj_wireframe.tga` | None |
| `--filled` | `obj_filled.tga` | `obj_zbuffer.tga` |

A bare filename is searched first as `obj/<filename>`, then as
`obj/<stem>/<filename>`. For example, `diablo3_pose.obj` resolves to
`obj/diablo3_pose/diablo3_pose.obj`. Use an explicit relative or absolute path
when a folder has a different name, such as `obj/boggie/head.obj`.
Relative paths are resolved from the terminal's working directory.

The shared OBJ reader handles vertex positions (`v`), slash-separated and
negative face indices, and polygonal faces. It projects x/y coordinates from
[-1, 1] to an 800 × 800 framebuffer without automatically fitting the model.
Wireframe mode draws white face boundaries. Filled mode splits convex faces
into triangle fans and assigns a random color to each triangle. Lighting,
textures, and perspective projection are not applied.

The public C functions replace the `ObjRenderer` class:

```c
#include "obj_renderer.h"

renderObjWireframe("diablo3_pose.obj", "obj_wireframe.tga");
renderObjFilled("diablo3_pose.obj", "obj_filled.tga", "obj_zbuffer.tga");
```

A NULL output path selects the corresponding default filename. For
`renderObjFilled`, NULL selects the default depth filename; `""` disables depth
output. Return values are 0 on success and 1 on input, allocation, or output failure.

Depth values are interpolated from OBJ z coordinates mapped from [-1, 1] to
[0, 255] and clamped before writing an 8-bit grayscale image. Larger values are
brighter; untouched pixels remain black. The current implementation preserves
the pre-migration ordering: the depth image retains the largest quantized z,
but the color is written **before** the depth comparison. Thus color visibility
still depends on face drawing order; this is not yet full hidden-surface removal.

## Viewing output and debugging

The color outputs are 24-bit RLE RGB TGA (Type 10). The depth image is 8-bit RLE
grayscale TGA (Type 11); some preview extensions do not support grayscale TGA.
[Xarmina TGA Viewer](https://marketplace.visualstudio.com/items?itemName=Xarmina.tga-preview)
lists Type 11 support. On macOS, a PNG preview can also be generated with:

```bash
sips -s format png framebuffer.tga --out framebuffer-preview.png
```

During debugging, continue past the drawing loops to `tgaImageWrite`; stopping
before that call leaves the previous file on disk. Rebuild after source changes.
Use a Debug build to inspect local variables and breakpoints in `src/*.c`.

## Tests

CTest builds the C regression test executable by default:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

Tests cover TGA formats/origins/RLE and invalid packets, line direction and
endpoints, scanline and barycentric rendering, color/depth interpolation,
shared OBJ parsing, and output failures. Fixtures are generated under
`build/test-output/`. Set `-DBUILD_TESTING=OFF` to build only the renderer.
With a multi-configuration generator, use `ctest --test-dir build -C Debug`.

## Random-line performance benchmark

```bash
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
./build-release/cpu_renderer --benchmark
```

The benchmark draws 16 million random lines in the 128 × 128 RGB framebuffer.
The C version uses `srand(42)` and `rand()` for coordinates and BGRA channels;
color channels range from 0 through 254. The sequence is repeatable with the
same C runtime but differs from the previous C++ `std::mt19937` sequence.
Timing uses C `clock()` and reports **CPU seconds**, including random generation
and line drawing but excluding the TGA write.

[BENCHMARKS.md](BENCHMARKS.md) preserves historical C++ measurements from
2026-10-06 on a 64 × 64 framebuffer. Those wall-time measurements and random
sequences are not directly comparable to the current C benchmark.

## Files

- `src/main.c`: Example scene, benchmark, and OBJ command-line entry point.
- `src/colors.c` / `colors.h`: Shared BGRA color constants.
- `src/line_renderer.c` / `line_renderer.h`: Line drawing.
- `src/triangle_renderer.c` / `triangle_renderer.h`: Scanlines, color interpolation,
  bounding boxes, and depth output.
- `src/obj_renderer.c` / `obj_renderer.h`: Model paths, shared parsing, and drawing modes.
- `third_party/tinyrenderer/`: Attributed C adaptation of TinyRenderer's TGA utilities.
- `tests/renderer_tests.c`: C API regression tests.
- `obj/`: Models and their associated textures.
- `CMakeLists.txt`: C17 build and test configuration.

## Progress

- Added pixel access, TGA output, line drawing, and integer error accumulation.
- Added scanline and bounding-box triangle rasterization with RGB interpolation.
- Added shared OBJ loading, wireframe/filled modes, model filename lookup, and depth output.
- Converted all implementation and reference sources from C++20 to C17, including TGA I/O.

The README is updated whenever features, build steps, or usage change.
