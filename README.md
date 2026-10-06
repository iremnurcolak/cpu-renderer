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
  greater change. It accumulates the absolute slope in `error` and adjusts the
  integer `y` coordinate whenever the error exceeds half a pixel. For steep
  lines, the x and y axes are swapped and restored when writing the pixel.
  Endpoints are ordered along the traversal axis, so lines supplied in reverse
  order color the same pixels.
- The example scene draws colored lines between three points and marks the
  endpoints in white. One line is drawn in both directions so their overlap can
  be inspected.

`main` currently uses `drawLineInterpolated`. The function supports horizontal,
vertical, and steep lines. Coincident endpoints produce a single pixel.

## Random-line performance benchmark

Historical measurements are stored in [BENCHMARKS.md](BENCHMARKS.md).

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

- `src/main.cpp`: Line-drawing functions and the example scene.
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
  runs under the same Release settings, the old method had a median time of
  2.384 seconds and the incremental method 2.391 seconds; no meaningful speed
  difference was observed. These times include random number generation and
  vary by system.
- Changed line rasterization to use an integer coordinate and accumulated error;
  detailed measurements are recorded in `BENCHMARKS.md`.

The README is updated whenever features, build steps, or usage change.
