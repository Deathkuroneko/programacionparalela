#include <cstdint>
#include <immintrin.h>
#include <omp.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void grayAVX(uint32_t *pixel, int heigth, int width) {
  int total = heigth * width;
  int i = 0;

  __m256 factR = _mm256_set1_ps(0.299f);
  __m256 factG = _mm256_set1_ps(0.72f);
  __m256 factB = _mm256_set1_ps(0.07f);
  __m256i mask = _mm256_set1_epi32(0xFF);

  for (; i <= total - 8; i += 8) {
    __m256i pixels = _mm256_loadu_si256((__m256i *)&pixel[i]);

    __m256i r = _mm256_and_si256(pixels, mask);

    __m256i g = _mm256_and_si256(_mm256_srli_epi32(pixels, 8), mask);
    __m256i b = _mm256_and_si256(_mm256_srli_epi32(pixels, 16), mask);
    __m256i a = _mm256_and_si256(_mm256_srli_epi32(pixels, 24), mask);

    //----------------------
    __m256 rf = _mm256_cvtepi32_ps(r);
    __m256 gf = _mm256_cvtepi32_ps(g);
    __m256 bf = _mm256_cvtepi32_ps(b);

    ///------------------------

    __m256 gray = _mm256_add_ps(
        _mm256_add_ps(_mm256_mul_ps(rf, factR), 
        _mm256_mul_ps(gf, factG)),
        _mm256_mul_ps(bf, factB));

    //------------------------
    __m256i grayI = _mm256_cvttps_epi32(gray);
    grayI = _mm256_min_epi32(grayI, _mm256_set1_epi32(255));
    //---------------------------------

    __m256i result = _mm256_or_si256(
        _mm256_slli_epi32(a, 24),
        _mm256_or_si256(_mm256_slli_epi32(grayI, 16),
                        _mm256_or_si256(_mm256_slli_epi32(grayI, 8), grayI)));
    _mm256_storeu_si256((__m256i *)&pixel[i], result);
  }
  for (; i < total; i++) {
    uint32_t pixels = pixel[i];
    uint32_t a = (pixels >> 24) & 0xFF;
    uint32_t b = (pixels >> 16) & 0xFF;
    uint32_t g = (pixels >> 8) & 0xFF;
    uint32_t r = (pixels >> 0) & 0xFF;

    uint32_t gray = uint32_t((0.299 * r) + (0.72 * g) + (0.07 * b));
    pixel[i] = (a << 24) | (gray << 16) | (gray << 8) | (gray << 0);
  }
  stbi_write_png("salida_simd.png", width, heigth, 4, pixel, width * 4);
}

void grayOPENMP(uint32_t *pixel, int heigth, int width) {
  int total = heigth * width;
  int totalThreads = omp_get_max_threads();
  int delta = (total + totalThreads - 1) / totalThreads;
#pragma omp parallel
  {
    int idThread = omp_get_thread_num();
    int start = delta * idThread;
    int end = (idThread == totalThreads - 1) ? total : (delta + start);

    for (int i = start; i < end; i++) {
      uint32_t pixels = pixel[i];
      uint32_t a = (pixels >> 24) & 0XFF;
      uint32_t r = (pixels >> 0) & 0XFF;
      uint32_t g = (pixels >> 8) & 0XFF;
      uint32_t b = (pixels >> 16) & 0XFF;

      int gray = (0.21 * r) + (0.72 * g) + (0.07 * b);

      uint32_t nPixel = (a << 24) | (gray << 16) | (gray << 8) | (gray);

      pixel[i] = nPixel;
    }
  }
  stbi_write_png("salida_openmp.png", width, heigth, 4, pixel, width * 4);
}

int main() {
  int heigth, width, chanels;
  uint8_t *rgba =
      stbi_load("perro.jpg", &width, &heigth, &chanels, STBI_rgb_alpha);
  if (!rgba) {
    return -1;
  }
  chanels = 4;
  int total = width * heigth;
  uint32_t *pixelBuffer = new uint32_t[total];
  uint32_t *pixelBuffer2 = new uint32_t[total];
  uint32_t *OriginalBuffer = new uint32_t[total];

  for (int i = 0; i < total; i++) {
    uint32_t r = rgba[i * 4 + 0];
    uint32_t g = rgba[i * 4 + 1];
    uint32_t b = rgba[i * 4 + 2];
    uint32_t a = rgba[i * 4 + 3];
    uint32_t px = (a << 24) | (b << 16) | (g << 8) | r;

    pixelBuffer[i] = px;
    pixelBuffer2[i] = px;
    OriginalBuffer[i] = px;
  }

  grayAVX(pixelBuffer, heigth, width);
  grayOPENMP(pixelBuffer2, heigth, width);
}
