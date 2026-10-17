# CPU Renderer

A renderer project for learning pixel, line, and triangle drawing on the CPU with C++20.
Image output is saved in TGA format using TinyRenderer's `TGAImage` class.

## Building and running

Requirements: CMake 3.20 or later and a compiler with C++20 support.

From the project root:

```bash
cmake -S . -B build
cmake --build build
./build/cpu_renderer
```

The first command generates the build files, and the second builds the program.
The program writes `framebuffer.tga` to its working directory. The generated
image and the `build/` directory are not tracked by Git.

## Current state

- A 128 × 128 RGB framebuffer is created for the example scene.
- `drawLineBarycentric` samples between two endpoints by incrementing `t` by
  `0.02`. Because it uses a fixed number of samples, long lines may contain
  gaps.
- `drawLineInterpolated` advances one pixel at a time along the axis with the
  greater change. It uses an integer error term scaled by two and adjusts the
  integer `y` coordinate whenever that term exceeds the distance along the
  traversal axis. This avoids floating-point arithmetic inside the loop. For
  steep lines, the x and y axes are swapped and restored when writing the
  pixel. Endpoints are ordered along the traversal axis, so lines supplied in
  reverse order color the same pixels.
- The example scene fills three counterclockwise triangles with bounding-box
  rasterization and interpolates the RGB colors assigned to its vertices.

`drawLineInterpolated` supports horizontal, vertical, and steep lines.
Coincident endpoints produce a single pixel.

## Rendering OBJ models

OBJ models are rendered through the same `cpu_renderer` executable. Place your
model in `obj/`, then build and run from the project root:

```bash
cmake -S . -B build
cmake --build build
./build/cpu_renderer diablo3_pose.obj
```

Select the drawing mode with `--wireframe` or `--filled`, before or after the
model path. Without a mode flag, filled rendering is used.

```bash
./build/cpu_renderer --wireframe diablo3_pose.obj
./build/cpu_renderer --filled obj/african_head/african_head.obj
```

Wireframe output is saved to `obj_wireframe.tga`; filled output is saved to
`obj_filled.tga`. Filled mode also saves an 800 × 800 grayscale depth image
to `obj_zbuffer.tga`. Mode flags require a model and cannot be combined with
each other or with `--benchmark`.

A filename without a directory is looked up first as `obj/<filename>`, then as
`obj/<stem>/<filename>`. For example, `diablo3_pose.obj` resolves to
`obj/diablo3_pose/diablo3_pose.obj`. Models stored in differently named folders
require an explicit path, such as `obj/boggie/head.obj`.
You can also supply an explicit relative or absolute path:

```bash
./build/cpu_renderer obj/diablo3_pose/diablo3_pose.obj
./build/cpu_renderer /absolute/path/to/model.obj
```

Relative paths, including `obj/`, are resolved from the terminal's
working directory. Run the commands above from the project root.

On Windows with a multi-configuration generator, build and run with:

```powershell
cmake --build build --config Release
.\build\Release\cpu_renderer.exe diablo3_pose.obj
```

The renderer reads vertex positions (`v`) and polygon faces (`f`), including
slash-separated face indices and negative vertex indices. The executable currently
fills each triangle with a random color on an 800 × 800 RGB image and writes
`obj_filled.tga` to the working directory. The terminal reports the loaded vertex
and face counts. Textures, lighting, and depth testing are not applied.
The x and y coordinates are mapped from the range [-1, 1] to the image; models
outside that range are not automatically fitted to the viewport.

`ObjRenderer` shares OBJ parsing, coordinate projection, and image output between
its two drawing modes:

```cpp
ObjRenderer renderer;
renderer.renderFilled("diablo3_pose.obj", "obj_filled.tga");
renderer.renderWireframe("diablo3_pose.obj", "obj_wireframe.tga");
```

Wireframe mode draws face boundaries in white. Filled mode splits polygonal
faces into a fan of triangles; this requires convex polygonal faces. Triangles
with signed area below 1 pixel squared are skipped, including clockwise and
degenerate triangles when y increases upward.

Filled mode interpolates OBJ vertex z values using the same barycentric weights
as the colors. Model z is mapped from [-1, 1] to [0, 255] and clamped when written:
larger z values appear brighter, and untouched pixels remain black. This is a
depth visualization only: no depth comparison is performed, so later faces
overwrite both color and depth at overlapping pixels. Back-face culling still
applies. Wireframe mode does not produce a depth image.

The depth output path can be changed through the C++ API:

```cpp
renderer.renderFilled("diablo3_pose.obj", "obj_filled.tga", "obj_zbuffer.tga");
```

Pass an empty third argument to disable depth output. The depth image uses
8-bit RLE grayscale TGA (Type 11), which requires a compatible viewer such as
[Xarmina TGA Viewer](https://marketplace.visualstudio.com/items?itemName=Xarmina.tga-preview).

## Triangle rendering

`src/triangle_renderer.h` declares three filled-triangle drawing functions:

- `drawTriangleWithMyScanlineV1`: Floating-point scanline intersections;
  vertex y coordinates must be distinct.
- `drawTriangleWithMyScanlineV2`: Integer scanline intersections.
- `drawTriangleWithBoundingBox`: Bounding-box traversal with barycentric inside
  tests and separate interpolation of the B, G, and R channels of three vertex
  colors (`colorA`, `colorB`, `colorC`). Output alpha is always 255.

```cpp
#include "triangle_renderer.h"
#include "colors.h"

TGAImage framebuffer(128, 128, TGAImage::RGB);
// Counterclockwise vertices, followed by their respective colors.
drawTriangleWithBoundingBox(7, 45, 45, 60, 35, 100, red, green, blue, framebuffer);
framebuffer.write_tga_file("framebuffer.tga");
```

Pass the same color for all three vertices to draw a uniformly colored triangle.
The OBJ renderer uses this to preserve its random color per triangle.

## Viewing output

The example writes a 24-bit, RLE-compressed RGB TGA (Type 10). Open
`framebuffer.tga` in a TGA viewer, such as a VS Code TGA preview extension.
A PNG preview can also be generated on macOS:

```bash
sips -s format png framebuffer.tga --out framebuffer-preview.png
```

The generated PNG is ignored by Git. During debugging, continue past the drawing
loops to `write_tga_file`; a breakpoint before that call leaves the previous file
on disk. Rebuild after changing the source before running the executable.

## Random-line performance benchmark

Historical measurements are stored in [BENCHMARKS.md](BENCHMARKS.md).

The final branchless integer-error implementation completed 16 million lines in
a median of **2.05886 seconds** across three Release runs on 2026-10-06. The
individual runs took 2.05021, 2.07506, and 2.05886 seconds.

```bash
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
./build-release/cpu_renderer --benchmark
```

`--benchmark` calls `drawLineInterpolated` exactly 16 million times. On each
call, endpoints are chosen randomly within the 128 × 128 image, and the BGRA
channels are generated directly at the call site with `std::rand() % 255`,
producing values from 0 through 254. The RGB framebuffer uses the three color
channels and ignores alpha. Coordinates use `std::mt19937(42)` and colors use `std::srand(42)`;
these fixed seeds make the coordinate and color sequences reproducible. The
reported time includes random coordinate and color generation as well as line
drawing, but excludes writing the TGA file. Running without arguments continues
to render the example scene.

## Files

- `src/main.cpp`: Example scene, line benchmark, and OBJ rendering entry point.
- `src/line_renderer.h`: Reusable line-drawing interface.
- `src/line_renderer.cpp`: Line-drawing implementations.
- `src/triangle_renderer.h`: Triangle-drawing interface.
- `src/triangle_renderer.cpp`: Scanline and bounding-box triangle rasterization.
- `src/obj_renderer.cpp`: Model path resolution, OBJ parsing, and wireframe or filled rendering.
- `obj/`: OBJ models and associated model textures, grouped in subfolders.
- `third_party/tinyrenderer/`: TGA image utilities.
- `CMakeLists.txt`: Build configuration.
- `BENCHMARKS.md`: Historical benchmark results.

## Progress

- Created the CPU framebuffer and pixel access.
- Added TinyRenderer TGA output.
- Added fixed-step interpolation and x-axis-based line drawing.
- Added support for steep lines by swapping axes and renamed the current
  function to `drawLineInterpolated`.
- Added support for coincident endpoints and a performance benchmark for 16
  million random lines.
- Replaced per-pixel `t` calculations with incremental slope updates. In three
  runs under the same Release settings on the same machine, the old method had
  a median time of 2.77594 seconds and the incremental method 2.05727 seconds.
  These times include random number generation and vary by system.
- Changed line rasterization to use an integer coordinate and accumulated error;
  the final version scales the error term and converts the threshold comparison
  to an integer multiplier, avoiding an explicit branch in the inner loop.
  Detailed measurements are recorded in `BENCHMARKS.md`.

- Added OBJ wireframe and filled rendering with shared parsing.
- Added per-vertex RGB color interpolation and a color-gradient triangle example.
- Moved model lookup to `obj/`, supporting both direct and model-named subfolders.

The README is updated whenever features, build steps, or usage change.
