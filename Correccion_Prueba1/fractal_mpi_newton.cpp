#include "fractal_mpi_newton.h"
#include "palette.h"
#include <cstdint>
#include <complex>
#include <cmath>
#include "mpi.h"

extern int max_iteraciones;


static const std::complex<double> raices[NEWTON_NUM_ROOTS] = {
    std::complex<double>(1.0, 0.0),
    std::complex<double>(-0.5, 0.8660254037844386), 
    std::complex<double>(-0.5, -0.8660254037844386),
};

static uint32_t acotado_newton(double x, double y, int &iteraciones_realizadas)
{
    std::complex<double> z(x, y);
    int iter = 0;
    int root_index = -1;

    while (iter < max_iteraciones)
    {
        double modulo_cuadrado = std::norm(z); 
        if (modulo_cuadrado < 1e-12)
        {
            iteraciones_realizadas = iter;
            return 0xFF000000; 
        }

        std::complex<double> z2 = z * z;
        std::complex<double> fz = z2 * z - 1.0; 
        std::complex<double> derivada_funcion_z = 3.0 * z2;  

        z = z - fz / derivada_funcion_z;
        iter++;     

        double z_modulo_cuadrado = std::norm(z);
        if (z_modulo_cuadrado > 4.0)
        {
            iteraciones_realizadas = iter;
            return 0xFF000000; // negro
        }

        // Criterio de convergencia
        for (int k = 0; k < NEWTON_NUM_ROOTS; k++)
        {
            std::complex<double> diff = z - raices[k];
            if (std::norm(diff) < NEWTON_EPS * NEWTON_EPS)
            {
                root_index = k;
                break;
            }
        }
        if (root_index != -1)
        {
            break;
        }
    }

    iteraciones_realizadas = iter;

    if (root_index == -1)
    {
        // En caso de no converger
        return 0xFF000000;
    }

    int index = (root_index * 5 + iter) % PALETTE_SIZE;
    return color_ramp[index];
}

void newton_mpi(double x_min, double y_min, double x_max, double y_max,
                 uint32_t width, uint32_t height,
                 uint32_t row_start, uint32_t row_end,
                 uint32_t *pixel_buffer,
                 double &compute_ms, long &total_iters)
{
    double dx = (x_max - x_min) / width;
    double dy = (y_max - y_min) / height;

    long iter_sum = 0;
    double t0 = MPI_Wtime();

    for (uint32_t j = row_start; j < row_end; j++)
    {
        for (uint32_t i = 0; i < width; i++)
        {
            // z0 = x + iy
            double x = x_min + i * dx;
            double y = y_max - j * dy;

            int iter = 0;
            uint32_t color = acotado_newton(x, y, iter);
            iter_sum += iter;

            pixel_buffer[(j - row_start) * width + i] = color;
        }
    }

    double t1 = MPI_Wtime();
    compute_ms = (t1 - t0) * 1000.0;
    total_iters = iter_sum;
}