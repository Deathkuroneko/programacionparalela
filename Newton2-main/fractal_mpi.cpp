#include "fractal_mpi.h"
#include "palette.h"
#include <cstdint>
#include <complex>

extern int max_iteraciones;

uint32_t acotado(double x, double y, long long &iters)
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

void newton_mpi(double x_min, double y_min, double x_max, double y_max,
               uint32_t width, uint32_t height, uint32_t row_start, uint32_t row_end,
               uint32_t *pixel_buffer,long long &total_iters)
{

    double dx = (x_max - x_min) / width;
    double dy = (y_max - y_min) / height;

    for (int j = row_start; j < row_end; j++)
    {
        for (int i = 0; i < width; i++)
        {

            double x = x_min + i * dx;
            double y = y_max - j * dy;

            auto color = acotado(x, y,total_iters);

            pixel_buffer[(j - row_start) * width + i] = color;
        }
    }

}