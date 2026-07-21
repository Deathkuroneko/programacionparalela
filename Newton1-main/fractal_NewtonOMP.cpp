#include "fractal_Newton.h"
#include "palette.h"
#include <cstdint>
#include <complex>
#include <omp.h>

extern int max_iteraciones;

uint32_t acotado_omp(double x, double y, long long &iters)
{

    int iter = 1;
    std::complex<double> z(x, y);
    std::complex<double> raices[3] =
        {
            {1.0, 0.0},
            {-0.5, sqrt(3) / 2},
            {-0.5, -sqrt(3) / 2}};

    while (iter < max_iteraciones)
    {
        auto f = z * z * z - 1.0;
        auto fDerivada = 3.0 * z * z;
        z = z - f / fDerivada;
        iter++;
        if (std::abs(z) > 2.0)
            return 0xFF000000;
        ;

        for (int root = 0; root < 3; root++)
        {
            if (std::abs(z - raices[root]) < 1e-4)
            {
                int index = (root * 5 + iter) % PALETTE_SIZE;

                return color_ramp[index];
            }
        }
        iter++;
    }

    iters += iter;
    return 0xFF000000;
}

void newton_omp(double x_min, double y_min, double x_max, double y_max,
                uint32_t width, uint32_t height,
                uint32_t *pixel_buffer, long long &total_iters)
{

    double dx = (x_max - x_min) / width;
    double dy = (y_max - y_min) / height;

#pragma omp parallel
    {
        int thread_count = omp_get_num_threads();
        int thread_id = omp_get_thread_num();
        int delta = std::ceil((width * 1.0 / thread_count));
        int start = thread_id * delta;
        int end = (thread_id + 1) * delta;
        if (thread_id == thread_count - 1)
        {
            end = width;
        }

        long long iters_local = 0;
        for (int i = start; i < end; i++)
        {
            for (int j = 0; j < height; j++)
            {

                double x = x_min + i * dx;
                double y = y_max - j * dy;

                auto color = acotado_omp(x, y, iters_local);

                pixel_buffer[j * width + i] = color;
            }
        }
#pragma omp critical
        {
            total_iters += iters_local;
        }
    }
}