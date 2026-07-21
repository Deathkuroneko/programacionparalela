#include "fractal_Burning.h"
#include "palette.h"

#include <cstring>
#include <complex>
#include <immintrin.h>
#include <vector>

extern int max_iteraciones;

void burning_simd(double x_min, double y_min, double x_max, double y_max,
                   uint32_t width, uint32_t height,
                   uint32_t *pixel_buffer, std::vector<int> &local_hist)
{

    double dx = (x_max - x_min) / width;
    double dy = (y_max - y_min) / height;
    __m256 xmin = _mm256_set1_ps(x_min);
    __m256 ymax = _mm256_set1_ps(y_max);

    __m256 xscale = _mm256_set1_ps(dx);
    __m256 yscale = _mm256_set1_ps(dy);

    __m256 max_norma = _mm256_set1_ps(4.0f);

    __m256 one = _mm256_set1_ps(1.0f);

    __m256 abs_mask = _mm256_castsi256_ps(_mm256_set1_epi32(0x7FFFFFFF));

    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j += 8)
        {
            __m256 mx = _mm256_set1_ps(i);
            __m256 my = _mm256_set_ps(j + 7, j + 6, j + 5, j + 4, j + 3, j + 2, j + 1, j + 0);
            __m256 cr = _mm256_add_ps(xmin, _mm256_mul_ps(mx, xscale));
            __m256 ci = _mm256_sub_ps(ymax, _mm256_mul_ps(my, yscale));
            int iter = 1;
            __m256 mk = _mm256_set1_ps(iter);

            __m256 zr = _mm256_setzero_ps();
            __m256 zi = _mm256_setzero_ps();

            while (iter < max_iteraciones)
            {

                __m256 zr_abs = _mm256_and_ps(zr, abs_mask);
                __m256 zi_abs = _mm256_and_ps(zi, abs_mask);

                __m256 zr2 = _mm256_mul_ps(zr_abs, zr_abs); 
                __m256 zi2 = _mm256_mul_ps(zi_abs, zi_abs);

                __m256 zrzi = _mm256_mul_ps(zr_abs, zi_abs);

                zr = _mm256_add_ps(_mm256_sub_ps(zr2, zi2), cr);
                zi = _mm256_add_ps(_mm256_add_ps(zrzi, zrzi), ci);

                zr2 = _mm256_mul_ps(zr, zr);
                zi2 = _mm256_mul_ps(zi, zi);

                __m256 norma2 = _mm256_add_ps(zr2, zi2);
                __m256 mask = _mm256_cmp_ps(norma2, max_norma, _CMP_LE_OS);

                mk = _mm256_add_ps(_mm256_and_ps(mask, one), mk);

                if (_mm256_testz_ps(mask, _mm256_set1_ps(-1)))
                {
                    break;
                }
                iter++;
            }

            float d[8];
            _mm256_storeu_ps(d, mk);
            for (int it = 0; it < 8; it++)
            {

                int index = (j + it) * width + i;
                if (index < width * height)
                {
                    int iter_final = (int)d[it];

                    int bin = iter_final * 16 / max_iteraciones;
                    if (bin >= 16)
                        bin = 15;

                    if (iter_final < max_iteraciones)
                    {
                        local_hist[bin]++;
                        int color_index = iter_final % PALETTE_SIZE;
                        pixel_buffer[index] = color_ramp[color_index];
                    }
                    else
                    {
                        pixel_buffer[index] = 0xFF000000;
                    }
                }
            }
        }
    }
}