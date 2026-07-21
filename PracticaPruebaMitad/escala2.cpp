#include <cstdint>
#include <immintrin.h>
#include <iostream>
#include <omp.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
void avxGray(uint32_t *pixelBuffer, int height, int width) {
  int total = height * width;
  __m256 facR = _mm256_set1_ps(0.21);
  __m256 facG = _mm256_set1_ps(0.72);
  __m256 facB = _mm256_set1_ps(0.07);
  __m256i mask = _mm256_set1_epi32(0xFF);
  int i = 0;
  for (; i < total - 8; i += 8) {
    __m256i pixel = _mm256_loadu_si256((__m256i *)&pixelBuffer[i]);
    __m256i r = _mm256_and_si256(pixel, mask);
    __m256i g = _mm256_and_si256(_mm256_srli_epi32(pixel, 8), mask);
    __m256i b = _mm256_and_si256(_mm256_srli_epi32(pixel, 16), mask);
    __m256i a = _mm256_and_si256(_mm256_srli_epi32(pixel, 24), mask);
    ///////////////////////
    __m256 rf = _mm256_cvtepi32_ps(r);
    __m256 gf = _mm256_cvtepi32_ps(g);
    __m256 bf = _mm256_cvtepi32_ps(b);

    ///////////////
    __m256 gray = _mm256_add_ps(
        _mm256_mul_ps(rf, facR),
        _mm256_add_ps(_mm256_mul_ps(gf, facG), _mm256_mul_ps(bf, facB)));
    /////////////
    __m256i grayI =
        _mm256_min_epi32(_mm256_cvttps_epi32(gray), _mm256_set1_epi32(255));

    //////////
    __m256i pixelN = _mm256_or_si256(
        grayI, _mm256_or_si256(_mm256_slli_epi32(grayI, 8),
                               _mm256_or_si256(_mm256_slli_epi32(grayI, 16),
                                               _mm256_slli_epi32(a, 24))));

    _mm256_storeu_si256((__m256i *)&pixelBuffer[i], pixelN);
  }

  for (; i < total; i++) {
    uint32_t pixel = pixelBuffer[i];
    uint32_t a = (pixel >> 24) & 0xFF;
    uint32_t b = (pixel >> 16) & 0xFF;
    uint32_t g = (pixel >> 8) & 0xFF;
    uint32_t r = (pixel >> 0) & 0xFF;

    uint32_t gray = (r * 0.21) + (g * 0.72) + (b * 0.07);

    uint32_t pixelN = (a << 24) | (gray << 16) | (gray << 8) | (gray);
    pixelBuffer[i] = pixelN;
  }

  stbi_write_png("salida_simd.png", width, height, 4, pixelBuffer, width * 4);
}



void openMPGray(uint32_t *pixelBuffer, int height, int width) {
  int num_Threads = omp_get_max_threads();
  int total = height * width;
  int delta = total / num_Threads;
#pragma omp parallel
  {
    int id_Thread = omp_get_thread_num();
    int start = (delta * id_Thread);
    int end = (id_Thread == num_Threads - 1) ? total : (delta + start);
    for (int i = start ; i < end; i++) {
      uint32_t pixel = pixelBuffer[i];
      uint32_t a = (pixel >> 24) & 0xFF;
      uint32_t b = (pixel >> 16) & 0xFF;
      uint32_t g = (pixel >> 8) & 0xFF;
      uint32_t r = (pixel >> 0) & 0xFF;

      uint32_t gray = uint32_t((r * 0.21) + (g * 0.72) + (b * 0.07));

      uint32_t pixelN = (a << 24) | (gray << 16) | (gray << 8) | (gray << 0);
      pixelBuffer[i] = pixelN;
    }
  }
  stbi_write_png("img_grisOpenMP.png", width, height, 4, pixelBuffer, width*4);
}

int main() {
  int total, width, height, channels;
  uint8_t *rgba_pixels =
      stbi_load("perro.jpg", &width, &height, &channels, STBI_rgb_alpha);

  if (!rgba_pixels) {
    std::cout << "error al cargar";
  }
  channels = 4;
  total = width * height;
  uint32_t *pixelAvx = new uint32_t[total];
  uint32_t *pixelOpenMP = new uint32_t[total];
  for (int i = 0; i < total; i++) {
    uint32_t r = rgba_pixels[i * 4 + 0];
    uint32_t g = rgba_pixels[i * 4 + 1];
    uint32_t b = rgba_pixels[i * 4 + 2];
    uint32_t a = rgba_pixels[i * 4 + 3];

    uint32_t pixel = (a << 24) | (b << 16) | (g << 8) | r;

    pixelAvx[i] = pixel;
    pixelOpenMP[i] = pixel;
  }
  // avxGray(pixelAvx, height, width);
  openMPGray(pixelOpenMP, height, width);
  return 0;
}