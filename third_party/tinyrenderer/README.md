# TinyRenderer starting point

Upstream: https://github.com/ssloy/tinyrenderer

The unmodified `tgaimage.h`, `tgaimage.cpp`, and `starting_point.cpp`
(upstream `main.cpp`) were downloaded from the starting point linked in
the upstream README:
https://github.com/ssloy/tinyrenderer/tree/706b2dfecff65daeb93de568ee2c2bd87f277860

`LICENSE.txt` was downloaded from upstream `master` on 2026-10-04;
the starting point snapshot does not contain a license file.

The project compiles `tgaimage.cpp` and exposes its header directory.
`starting_point.cpp` is a reference example and is not compiled into
`cpu_renderer`, which uses `src/main.cpp`.

Usage notes:
- Include `"tgaimage.h"` to use `TGAImage` and `TGAColor`.
- `TGAColor` stores channels in BGRA order.
- `write_tga_file` defaults to `vflip=true` (bottom-left image origin).
  The starting point uses this default. Use `vflip=false` if you choose
  a top-left coordinate origin.
