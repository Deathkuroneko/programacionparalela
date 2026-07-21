#include "fractal_Newton.h"
#include "palette.h"

#include <cstring>
#include <complex>
#include <immintrin.h>
#include <cmath>

extern int max_iteraciones;

void newton_simd(double x_min, double y_min, double x_max, double y_max,
                  uint32_t width, uint32_t height,
                  uint32_t *pixel_buffer, long long &total_iters)
{

    double dx = (x_max - x_min) / width;
    double dy = (y_max - y_min) / height;

    __m256 xmin = _mm256_set1_ps(x_min);
    __m256 ymax = _mm256_set1_ps(y_max);

    __m256 xscale = _mm256_set1_ps(dx);
    __m256 yscale = _mm256_set1_ps(dy);

    __m256 one = _mm256_set1_ps(1.0f);
    __m256 three = _mm256_set1_ps(3.0f);
    __m256 radio2 = _mm256_set1_ps(4.0f);
    __m256 tol2 = _mm256_set1_ps(1e-4f * 1e-4f); // distancia^2 (evita sqrt)

    // 3 raices de z^3 = 1
    __m256 raiz_r[3] = {
        _mm256_set1_ps(1.0f),
        _mm256_set1_ps(-0.5f),
        _mm256_set1_ps(-0.5f)};
    __m256 raiz_i[3] = {
        _mm256_set1_ps(0.0f),
        _mm256_set1_ps((float)(sqrt(3.0) / 2.0)),
        _mm256_set1_ps((float)(-sqrt(3.0) / 2.0))};

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

            // z inicial = c (no cero, a diferencia de Burning)
            __m256 zr = cr;
            __m256 zi = ci;

            __m256 diverged_flag = _mm256_setzero_ps(); // 0 = no diverguio, 1 = diverguio
            __m256 root_val = _mm256_set1_ps(-1.0f);     // -1 = todavia no converge

            while (iter < max_iteraciones)
            {
                // z^2 = (zr+zi*i)*(zr+zi*i)
                __m256 z2r = _mm256_sub_ps(_mm256_mul_ps(zr, zr), _mm256_mul_ps(zi, zi));
                __m256 zrzi = _mm256_mul_ps(zr, zi);
                __m256 z2i = _mm256_add_ps(zrzi, zrzi);

                // z^3 = z^2 * z
                __m256 z3r = _mm256_sub_ps(_mm256_mul_ps(z2r, zr), _mm256_mul_ps(z2i, zi));
                __m256 z3i = _mm256_add_ps(_mm256_mul_ps(z2r, zi), _mm256_mul_ps(z2i, zr));

                // f = z^3 - 1
                __m256 fr = _mm256_sub_ps(z3r, one);
                __m256 fi = z3i;

                // f' = 3*z^2
                __m256 fder_r = _mm256_mul_ps(three, z2r);
                __m256 fder_i = _mm256_mul_ps(three, z2i);

                // q = f / f'  (division compleja hecha a mano)
                __m256 denom = _mm256_add_ps(_mm256_mul_ps(fder_r, fder_r), _mm256_mul_ps(fder_i, fder_i));
                __m256 qr = _mm256_div_ps(_mm256_add_ps(_mm256_mul_ps(fr, fder_r), _mm256_mul_ps(fi, fder_i)), denom);
                __m256 qi = _mm256_div_ps(_mm256_sub_ps(_mm256_mul_ps(fi, fder_r), _mm256_mul_ps(fr, fder_i)), denom);

                // z = z - q  (paso de Newton)
                zr = _mm256_sub_ps(zr, qr);
                zi = _mm256_sub_ps(zi, qi);

                // --- chequeo de divergencia |z|^2 > 4 ---
                __m256 mag2 = _mm256_add_ps(_mm256_mul_ps(zr, zr), _mm256_mul_ps(zi, zi));
                __m256 diverge_cmp = _mm256_cmp_ps(mag2, radio2, _CMP_GT_OS);
                __m256 diverge_now = _mm256_and_ps(diverge_cmp, one); // 1.0 si diverge ahora, 0.0 si no

                // una vez que diverge, se queda marcado (max con lo anterior)
                diverged_flag = _mm256_max_ps(diverged_flag, diverge_now);

                // --- chequeo de convergencia a cada una de las 3 raices ---
                for (int root = 0; root < 3; root++)
                {
                    __m256 dr = _mm256_sub_ps(zr, raiz_r[root]);
                    __m256 di = _mm256_sub_ps(zi, raiz_i[root]);
                    __m256 dist2 = _mm256_add_ps(_mm256_mul_ps(dr, dr), _mm256_mul_ps(di, di));

                    __m256 close = _mm256_cmp_ps(dist2, tol2, _CMP_LT_OS);

                    // si esta cerca: candidato = root ; si no: candidato = -1
                    __m256 id_mas_uno = _mm256_set1_ps((float)(root + 1));
                    __m256 candidato = _mm256_sub_ps(_mm256_and_ps(close, id_mas_uno), one);

                    // se queda con el primer root al que convergio (el mayor, ya que -1 < 0,1,2)
                    root_val = _mm256_max_ps(root_val, candidato);
                }

                // --- sigue activo si no diverguio y todavia no convergio ---
                __m256 no_diverguio = _mm256_cmp_ps(diverged_flag, _mm256_setzero_ps(), _CMP_EQ_OS);
                __m256 no_convergio = _mm256_cmp_ps(root_val, _mm256_setzero_ps(), _CMP_LT_OS);
                __m256 activo_mask = _mm256_and_ps(no_diverguio, no_convergio);

                // mk (contador de iteracion) solo avanza en los carriles activos
                mk = _mm256_add_ps(_mm256_and_ps(activo_mask, one), mk);

                // si ya no queda ningun carril activo, cortamos
                if (_mm256_testz_ps(activo_mask, _mm256_set1_ps(-1)))
                {
                    break;
                }

                iter++;
            }

            float d_iter[8];
            float d_root[8];
            float d_div[8];
            _mm256_storeu_ps(d_iter, mk);
            _mm256_storeu_ps(d_root, root_val);
            _mm256_storeu_ps(d_div, diverged_flag);

            for (int it = 0; it < 8; it++)
            {

                int index = (j + it) * width + i;
                if (index < width * height)
                {
                    int iter_final = (int)d_iter[it];
                    int root_final = (int)d_root[it];
                    bool diverged = d_div[it] > 0.5f;

                    if (root_final >= 0)
                    {
                        // convergio a una raiz
                        int color_index = (root_final * 5 + iter_final) % PALETTE_SIZE;
                        pixel_buffer[index] = color_ramp[color_index];
                    }
                    else if (diverged)
                    {
                        pixel_buffer[index] = 0xFF000000;
                    }
                    else
                    {
                        // nunca diverge ni converge -> se acaban las iteraciones
                        total_iters += iter_final;
                        pixel_buffer[index] = 0xFF000000;
                    }
                }
            }
        }
    }
}