# Benchmark Sonuçları

Benchmark, `drawLineInterpolated` fonksiyonunu 64 × 64 framebuffer üzerinde
16 milyon rastgele çizgi için çalıştırır. Koordinat ve renk üretimi ölçüm
süresine dahildir; TGA dosyasının diske yazılması dahil değildir.

| Tarih | Yöntem | Koşular (s) | Ortanca (s) |
|---|---|---:|---:|
| Önceki ölçüm | Her pikselde `t` hesaplama | Ham değerler kaydedilmemiş | 2,384 |
| Önceki ölçüm | `float y` değerini eğimle artırma | Ham değerler kaydedilmemiş | 2,391 |
| 2026-10-06 | Tamsayı `y` ve birikimli `error` | 2,55459 / 2,51690 / 2,50517 | 2,51690 |
| 2026-10-06 | Ölçeklenmiş tamsayı `ierror` (final) | 2,42865 / 2,37075 / 2,38202 | 2,38202 |

Son ölçüm Windows'ta Visual Studio 17 2022 CMake generator'ı ile Release
yapılandırmasında alındı. Sonuçlar sistem yüküne ve donanıma göre
değişebileceği için karşılaştırmalarda ortanca değer kullanılır.

## Çalıştırma

```powershell
cmake -S . -B build
cmake --build build --config Release
.\build\Release\cpu_renderer.exe --benchmark
```
