#include "fractal_mpi.h"
#include "palette.h"
#include <cstdint>
#include <complex>

extern int max_iteraciones;
extern std::complex<double> c;

uint32_t acotado_2(double x, double y)
{
    /*
    dados: c, z0
    Zn+1 = Zn^2 + c
    */

    int iter = 1;

    double zr = x;
    double zi = y;

    while (iter < max_iteraciones && (zr * zr + zi * zi) < 4.0)
    {
        double dr = zr * zr - zi * zi + c.real();
        double di = 2.0 * zr * zi + c.imag();
        zr = dr;
        zi = di;

        iter++;
    }
    if (iter < max_iteraciones)
    {
        // nomras > 2
        int index = iter % PALETTE_SIZE;
        return color_ramp[index];
    }

    // los bits esta alreves en cuanto a los colores
    return 0xFF000000; // negro
}

void julia_mpi(double x_min, double y_min, double x_max, double y_max, 
    uint32_t width, uint32_t height,uint32_t row_start, uint32_t row_end, 
    uint32_t *pixel_buffer)
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
            auto color = acotado_2(x, y);

            // index j*w + i
            pixel_buffer[(j - row_start) * width + i] = color;
        }
    }
    for (int i = 0; i < width; i++)
    {
    pixel_buffer[i] = 0xFF000000; // rojo para debug
    }
    
}