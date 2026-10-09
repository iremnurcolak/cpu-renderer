# CPU Renderer

A renderer project for learning pixel and line drawing on the CPU with C++20.
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

- A 64 × 64 RGB framebuffer is created.
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
- The example scene draws colored lines between three points and marks the
  endpoints in white. One line is drawn in both directions so their overlap can
  be inspected.

`main` currently uses `drawLineInterpolated`. The function supports horizontal,
vertical, and steep lines. Coincident endpoints produce a single pixel.

## Rendering OBJ models

OBJ models are rendered through the same `cpu_renderer` executable. Place your
model in `assets/models/`, then build and run from the project root:

```bash
cmake -S . -B build
cmake --build build
./build/cpu_renderer diablo3_pose.obj
```

A filename without a directory is resolved inside `assets/models/`. You can
also supply an explicit relative or absolute path:

```bash
./build/cpu_renderer assets/models/diablo3_pose.obj
./build/cpu_renderer /absolute/path/to/model.obj
```

Relative paths, including `assets/models/`, are resolved from the terminal's
working directory. Run the commands above from the project root.

On Windows with a multi-configuration generator, build and run with:

```powershell
cmake --build build --config Release
.\build\Release\cpu_renderer.exe diablo3_pose.obj
```

The renderer reads vertex positions (`v`) and polygon faces (`f`), including
slash-separated face indices and negative vertex indices. It draws each face's
outline in white on an 800 × 800 RGB image and writes `obj_framebuffer.tga` to
the working directory. The terminal reports the loaded vertex and face counts.
This is a wireframe render: textures, lighting, and filled faces are not used.
The x and y coordinates are mapped from the range [-1, 1] to the image; models
outside that range are not automatically fitted to the viewport.

## Triangle rendering scaffold

`drawTriangle` is declared in `src/triangle_renderer.h`, with an unfinished
implementation in `src/triangle_renderer.cpp`. It takes three 2D endpoints,
a framebuffer, and a color, matching the line-rendering interface. It is
included in the `renderer` library but does not draw any pixels yet.

To call it after implementing the function:

```cpp
#include "triangle_renderer.h"

// Inside a function with an existing framebuffer and color:
drawTriangle(7, 3, 12, 37, 62, 53, framebuffer, color);
```

The implementation contains TODOs for degenerate triangles, framebuffer bounds,
inside-triangle detection, and pixel writes.

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
call, endpoints are chosen randomly within the 64 × 64 image, and the BGRA
channels are generated directly at the call site with `std::rand() % 255`,
producing values from 0 through 254. The RGB framebuffer does not use the alpha
channel. Coordinates use `std::mt19937(42)` and colors use `std::srand(42)`;
these fixed seeds make the coordinate and color sequences reproducible. The
reported time includes random coordinate and color generation as well as line
drawing, but excludes writing the TGA file. Running without arguments continues
to render the example scene.

## Files

- `src/main.cpp`: Example scene, line benchmark, and OBJ rendering entry point.
- `src/line_renderer.h`: Reusable line-drawing interface.
- `src/line_renderer.cpp`: Line-drawing implementations.
- `src/triangle_renderer.h`: Triangle-drawing interface.
- `src/triangle_renderer.cpp`: Unfinished triangle rasterization scaffold.
- `src/obj_renderer.cpp`: Model path resolution, OBJ parsing, and wireframe rendering.
- `assets/models/`: OBJ model files used as renderer input.
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

- Added OBJ wireframe rendering and filename lookup inside `assets/models/`.

The README is updated whenever features, build steps, or usage change.
