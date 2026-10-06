# Benchmark Results

The benchmark runs `drawLineInterpolated` for 16 million random lines on a
64 × 64 framebuffer. Coordinate and color generation are included in the
measured time; writing the TGA file to disk is excluded.

| Date | Method | Runs (s) | Median (s) |
|---|---|---:|---:|
| Previous measurement | Per-pixel `t` calculation | Raw values were not recorded | 2.384 |
| Previous measurement | Incrementing `float y` by the slope | Raw values were not recorded | 2.391 |
| 2026-10-06 | Integer `y` with accumulated `error` | 2.55459 / 2.51690 / 2.50517 | 2.51690 |
| 2026-10-06 | Scaled integer `ierror` (final) | 2.42865 / 2.37075 / 2.38202 | 2.38202 |

The latest measurements were taken on Windows in a Release configuration using
the Visual Studio 17 2022 CMake generator. Results may vary with system load and
hardware, so comparisons use the median value.

## Running the benchmark

```powershell
cmake -S . -B build
cmake --build build --config Release
.\build\Release\cpu_renderer.exe --benchmark
```
