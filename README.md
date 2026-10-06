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
- `drawLineBarycentric`, iki uç nokta arasında `t` değerini `0.02` artırarak örnekleme yapıyor. Sabit örnek sayısı nedeniyle uzun çizgilerde boşluklar oluşabilir.
- `drawLineInterpolated`, çizginin daha fazla değişen ekseninde birer piksel ilerleyip diğer koordinatı doğrusal interpolasyonla hesaplıyor. Dik eğimli (steep) çizgilerde x ve y eksenleri yer değiştiriyor; piksel yazılırken özgün koordinat sırası geri kullanılıyor. Uç noktalar işleme eksenine göre sıralandığı için ters yönde verilen çizgiler de aynı pikselleri boyuyor.
- Örnek sahnede üç nokta arasında renkli çizgiler çiziliyor; uç noktalar beyaz ile işaretleniyor. Bir çizgi iki yönde çizilerek üst üste gelmesi inceleniyor.

`main` şu anda `drawLineInterpolated` fonksiyonunu kullanıyor. Fonksiyon yatay, dikey ve dik eğimli çizgileri destekliyor. İki uç nokta aynıysa tek piksel boyanıyor.

## Rastgele çizgi performans denemesi

```bash
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
./build-release/cpu_renderer --benchmark
```

`--benchmark`, `drawLineInterpolated` fonksiyonunu tam 16 milyon kez çağırır. Her çağrıda uç noktalar 64 × 64 görüntünün sınırları içinde rastgele seçilir; her çizginin BGRA kanalları doğrudan çağrı içinde `std::rand() % 255` ile 0–254 aralığından seçilir. RGB framebuffer alfa kanalını kullanmaz. Koordinatlar için `std::mt19937(42)`, renkler için `std::srand(42)` kullanılır; sabit seed değerleri sayesinde aynı rastgele koordinat ve renk dizisi tekrar kullanılabilir. Terminalde yazılan süre, rastgele koordinat ve renk üretimini ve çizgi çizimini kapsar; TGA dosyasının kaydedilmesini kapsamaz. Parametresiz çalıştırma örnek sahneyi çizmeye devam eder.

## Dosyalar

- `src/main.cpp`: Çizgi çizme fonksiyonları ve örnek sahne.
- `third_party/tinyrenderer/`: TGA görüntü yardımcıları.
- `CMakeLists.txt`: Derleme yapılandırması.

## İlerleme

- CPU framebuffer ve piksel erişimi oluşturuldu.
- TinyRenderer TGA çıktısı eklendi.
- Sabit adımlı interpolasyon ve x koordinatını kullanan çizgi çizimi eklendi.
- Eksenleri değiştirerek dik eğimli çizgiler desteklendi; güncel fonksiyon `drawLineInterpolated` olarak adlandırıldı.
- Aynı uç noktalar desteklendi ve 16 milyon rastgele çizgi için performans denemesi eklendi.

README, yeni özellikler ve derleme ya da kullanım değişiklikleriyle birlikte güncellenir.
