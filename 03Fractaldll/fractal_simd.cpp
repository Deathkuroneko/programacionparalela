#include "fractal_simd.h"
#include "palette.h"

#include <cstring>
#include <complex>
#include <immintrin.h> //LIBRERIA PARA AVX

std::complex<double> c(-0.7, 0.27015);

// no pueden existir mas de dos funciones

extern "C" _stdcall
void julia_simd(double x_min, double y_min, double x_max, double y_max, uint32_t width, uint32_t height, 
    int max_iteraciones, uint32_t *pixel_buffer)
{
    double dx = (x_max - x_min) / width;
    double dy = (y_max - y_min) / height;

    // genera un vector de 8 veces x_min eb base al tamaño de la unidad vectorial
    //(-1.5,-1.5,....)
    __m256 xmin = _mm256_set1_ps(x_min); // genera el vector
                                         //(1,1.1,....)
    __m256 ymax = _mm256_set1_ps(y_max);

    __m256 xscale = _mm256_set1_ps(dx);
    __m256 yscale = _mm256_set1_ps(dy);

    __m256 c_real = _mm256_set1_ps(c.real());
    __m256 c_imag = _mm256_set1_ps(c.imag());

    __m256 max_norma = _mm256_set1_ps(4.0f);

    __m256 one = _mm256_set1_ps(1.0f); //(1,1,1,1,1,1,1,1)
    
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j += 8)
        {
            __m256 mx = _mm256_set1_ps(i);
            __m256 my = _mm256_set_ps(j + 7, j + 6, j + 5, j + 4, j + 3, j + 2, j + 1, j + 0);
            // xmin+mx*xscale  -->(x0,x1...,x8) real
            __m256 cr = _mm256_add_ps(xmin, _mm256_mul_ps(mx, xscale)); // 8 partes reales de los complejos

            // ymax-my*yscale  -->(y0,y1...,y8) complejo/imaginario
            __m256 ci = _mm256_sub_ps(ymax, _mm256_mul_ps(my, yscale)); // 8 partes reales de los complejos

            // verificar si los 8 complejos (cr,ci) estan acotados o no
            int iter = 1;
            __m256 mk = _mm256_set1_ps(iter); // 8 iter

            __m256 zr = cr;
            __m256 zi = ci;

            while (iter < max_iteraciones)
            {
                // Zn+1 = Zn^2 + c

                __m256 zr2 = _mm256_mul_ps(zr, zr); //zr^2
                __m256 zi2 = _mm256_mul_ps(zi, zi); //zi^2

                __m256 zrzi = _mm256_mul_ps(zr,zi); //zr*zi

                zr = _mm256_add_ps(_mm256_sub_ps(zr2,zi2), c_real); //zr^2 - zi^2 +creal
                zi = _mm256_add_ps( _mm256_add_ps(zrzi,zrzi),c_imag); // 2*zr*zi + cimg

                //--normas

                zr2 = _mm256_mul_ps(zr,zr); //zr^2

                zi2 = _mm256_mul_ps(zi,zi); //zi^2

                __m256 norma2 =  _mm256_add_ps(zr2,zi2); //nomra^2

                //comparacion si norma2<4.0f devuelve 0xFFFFFFFF, sino 0x00000000
                __m256 mask = _mm256_cmp_ps(norma2,max_norma,_CMP_LE_OS); //COMPARA SI LA NORMA ES MENOR QUE LA NORMA MAX
                
                mk = _mm256_add_ps(_mm256_and_ps(mask,one),mk);

                if (_mm256_testz_ps(mask,_mm256_set1_ps(-1)))
                {
                    break;
                }
                iter++;
            }

            float d[8];
            _mm256_storeu_ps(d,mk);
            for (int it = 0; it < 8; it++)
            {

                int index =(j+it)*width+i;
                if (index < width*height)
                {
                    if (d[it]<max_iteraciones)
                    {
                        int color_index = (int)d[it]%PALETTE_SIZE;
                        pixel_buffer[index] = color_ramp[color_index];

                    }else{
                        pixel_buffer[index] = 0xFF000000;
                    }
                    
                }
                
            }
            

        }
    }
}
