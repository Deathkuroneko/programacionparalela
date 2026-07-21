#include "escala.h"

#include <immintrin.h>
#include <cstdint>
#include <omp.h>
//_mm256_set1_ps -> almacenar un valor float en todos los elementos de un registro AVX
//_mm256_set1_epi32 -> almacenar un valor entero en todos los elementos de un registro AVX
//_mm256_loadu_si256 -> cargar 8 enteros de 32 bits desde memoria a un registro AVX
//_mm256_and_si256 -> operación AND bit a bit entre dos registros AVX
//_mm256_srli_epi32 -> desplazar a la derecha los bits de cada entero
//_mm256_cvtepi32_ps -> convertir enteros de 32 bits a flotantes de 32 bits
//_mm256_mul_ps -> multiplicar dos registros AVX de flotantes
//_mm256_add_ps -> sumar dos registros AVX de flotantes
//_mm256_cvttps_epi32 -> convertir flotantes de 32 bits a enteros de 32 bits con truncamiento
//_mm256_slli_epi32 -> desplazar a la izquierda los bits de cada entero
//_mm256_or_si256 -> operación OR bit a bit entre dos registros AVX
//_mm256_storeu_si256 -> almacenar 8 enteros de 32 bits desde un registro AVX a memoria
//(__m256i*)&pixel_buffer[i] hacer un cast del puntero a un puntero a __m256i para cargar 8 píxeles a la vez



// ======================================================
// SIMD (AVX)
// ======================================================
void Gray_SIMD(uint32_t *pixel_buffer, int ancho, int alto)
{
    int total = ancho * alto;
    int i = 0;

    __m256 rFactor = _mm256_set1_ps(0.299f);
    __m256 gFactor = _mm256_set1_ps(0.72f);
    __m256 bFactor = _mm256_set1_ps(0.07f);

    __m256i mask = _mm256_set1_epi32(0xFF); 

    for (; i <= total - 8; i += 8)
    {
        __m256i pixels =
            _mm256_loadu_si256((__m256i*)&pixel_buffer[i]);

        // ======================================================
        // EXTRAER CANALES
        // Formato usado:
        // A B G R
        // ======================================================

        __m256i r =
            _mm256_and_si256(pixels, mask);

        __m256i g =
            _mm256_and_si256(
                _mm256_srli_epi32(pixels, 8),
                mask);

        __m256i b =
            _mm256_and_si256(
                _mm256_srli_epi32(pixels, 16),
                mask);

        __m256i a =
            _mm256_and_si256(
                _mm256_srli_epi32(pixels, 24),
                mask);

        // ======================================================
        // INT -> FLOAT
        // ======================================================

        __m256 rf = _mm256_cvtepi32_ps(r);
        __m256 gf = _mm256_cvtepi32_ps(g);
        __m256 bf = _mm256_cvtepi32_ps(b);

        // ======================================================
        // GRAYSCALE
        // ======================================================

        __m256 gray =
            _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(rf, rFactor),_mm256_mul_ps(gf, gFactor)),_mm256_mul_ps(bf, bFactor));

        __m256i gray_i =
            _mm256_cvttps_epi32(gray);

        // ======================================================
        // RECONSTRUIR PIXEL
        // Formato:
        // A B G R
        // ======================================================

        __m256i result =
            _mm256_or_si256(
                _mm256_slli_epi32(a, 24),

                _mm256_or_si256(
                    _mm256_slli_epi32(gray_i, 16),

                    _mm256_or_si256(
                        _mm256_slli_epi32(gray_i, 8),
                        gray_i)));

        _mm256_storeu_si256(
            (__m256i*)&pixel_buffer[i],
            result);
    }

    // ======================================================
    // SOBRANTES
    // ======================================================

    for (; i < total; i++)
    {
        uint32_t pixel = pixel_buffer[i];

        // Formato A B G R
        uint8_t r = pixel & 0xFF;
        uint8_t g = (pixel >> 8) & 0xFF;
        uint8_t b = (pixel >> 16) & 0xFF;
        uint8_t a = (pixel >> 24) & 0xFF;

        uint8_t gray =
            (uint8_t)(0.299f * r +
                       0.72f * g +
                       0.07f * b);

        pixel_buffer[i] =
            (a << 24) |
            (gray << 16) |
            (gray << 8) |
            gray;
    }
}

// ======================================================
// OPENMP
// ======================================================
void Gray_OPENMP(uint32_t *pixel_buffer, int ancho, int alto)
{
    int total = ancho * alto;

    const float rFactor = 0.299f;
    const float gFactor = 0.72f;
    const float bFactor = 0.07f;

    int num_threads = omp_get_max_threads();

    int delta = total / num_threads;

#pragma omp parallel
    {
        int threadId = omp_get_thread_num();

        int start = threadId * delta;

        int end =(threadId == num_threads - 1) ? total: start + delta;

        for (int i = start; i < end; i++)
        {
            uint32_t pixel = pixel_buffer[i];

            // Formato A B G R
            uint8_t r = pixel & 0xFF;
            uint8_t g = (pixel >> 8) & 0xFF;
            uint8_t b = (pixel >> 16) & 0xFF;
            uint8_t a = (pixel >> 24) & 0xFF;

            uint8_t gray =
                (uint8_t)(rFactor * r +
                           gFactor * g +
                           bFactor * b);

            pixel_buffer[i] =
                (a << 24) |
                (gray << 16) |
                (gray << 8) |
                gray;
        }
    }
}