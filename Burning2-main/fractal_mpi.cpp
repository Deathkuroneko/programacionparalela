#include "fractal_mpi.h"
#include "palette.h"
#include <cstdint>
#include <complex>
#include <iostream>

extern int max_iteraciones;

uint32_t acotado_2(double x, double y, std::vector<int> &local_hist)
{

    int iter = 1;

    double zr = 0.0;
    double zi = 0.0;
    double ci = y;
    double cr = x;

    while (iter < max_iteraciones && (zr * zr + zi * zi) < 4.0)
    {
        double zr_abs = std::abs(zr);
        double zi_abs = std::abs(zi);

        double nuevoReal = zr_abs * zr_abs - zi_abs * zi_abs + cr;
        double nuevoImag = (2 * zr_abs * zi_abs) + ci;

        zr = nuevoReal;
        zi = nuevoImag;

        iter++;
    }
    int bin = iter * 16 / max_iteraciones;
    if (bin >= 16)
        bin = 15;
    if (iter < max_iteraciones)
    {

        local_hist[bin]++;
        // nomras > 2
        int index = iter % PALETTE_SIZE;
        return color_ramp[index];
    }

    // los bits esta alreves en cuanto a los colores
    return 0xFF000000; // negro
}

void burning_mpi(double x_min, double y_min, double x_max, double y_max,
                 uint32_t width, uint32_t height, uint32_t row_start, uint32_t row_end,
                 uint32_t *pixel_buffer, std::vector<int> &local_hist)
{

    double dx = (x_max - x_min) / width;
    double dy = (y_max - y_min) / height;

    for (int j = row_start; j < row_end; j++)
    {
        for (int i = 0; i < width; i++)
        {

            // z = x+yi = (x,y)
            double x = x_min + i * dx;
            double y = y_max - j * dy;

            // similar al var
            auto color = acotado_2(x, y, local_hist);

            // index j*w + i
            pixel_buffer[(j - row_start) * width + i] = color;
        }
    }
}