# CPU Renderer

C++20 ile CPU üzerinde piksel ve çizgi çizimini öğrenmek için geliştirilen bir renderer projesi. Görüntü çıktısı, TinyRenderer'ın `TGAImage` sınıfı ile TGA formatında kaydedilir.

## Derleme ve çalıştırma

Gereksinimler: CMake 3.20 veya üzeri ve C++20 destekleyen bir derleyici.

Projenin kök klasöründe:

```bash
cmake -S . -B build
cmake --build build
./build/cpu_renderer
```

İlk komut derleme dosyalarını hazırlar, ikinci komut programı derler. Program, çalıştırıldığı klasöre `framebuffer.tga` dosyasını yazar. Üretilen görüntü ve `build/` klasörü Git'e dahil edilmez.

## Mevcut durum

- 64 × 64 piksel RGB framebuffer oluşturuluyor.
- `lineBarycentric`, iki uç nokta arasında `t` değerini `0.02` artırarak örnekleme yapıyor. Sabit örnek sayısı nedeniyle uzun çizgilerde boşluklar oluşabilir.
- `lineUsingXAsT`, uç noktaları x koordinatına göre sıralayıp her x sütunu için y koordinatını hesaplıyor.
- Örnek sahnede üç nokta arasında renkli çizgiler çiziliyor; uç noktalar beyaz ile işaretleniyor. Bir çizgi iki yönde çizilerek üst üste gelmesi inceleniyor.

`main` şu anda `lineUsingXAsT` fonksiyonunu kullanıyor. Bu yaklaşım dik çizgilerde sıfıra bölme sorununa yol açar ve dik eğimli çizgilerde y yönünde piksel atlayabilir; tüm eğimleri kapsayan çizgi çizimi henüz tamamlanmadı.

## Dosyalar

- `src/main.cpp`: Çizgi çizme fonksiyonları ve örnek sahne.
- `third_party/tinyrenderer/`: TGA görüntü yardımcıları.
- `CMakeLists.txt`: Derleme yapılandırması.

## İlerleme

- CPU framebuffer ve piksel erişimi oluşturuldu.
- TinyRenderer TGA çıktısı eklendi.
- Sabit adımlı interpolasyon ve x koordinatını kullanan çizgi çizimi eklendi.

README, yeni özellikler ve derleme ya da kullanım değişiklikleriyle birlikte güncellenir.
