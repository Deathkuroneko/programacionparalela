#include "fractal_mpi_burningShip.h"
#include "palette.h"
#include <cstdint>
#include <cmath>
#include <cstring>
#include "mpi.h"

extern int max_iteraciones;

static uint32_t burning_ship_pixel(double cx, double cy, int &iter_out, bool &escaped_out)
{
    double zr = 0.0;
    double zi = 0.0;
    int iter = 0;

    while (iter < max_iteraciones)
    {
        // Valor absoluto por componente
        double zr_abs = std::fabs(zr);
        double zi_abs = std::fabs(zi);

        double new_zr = zr_abs * zr_abs - zi_abs * zi_abs + cx;
        double new_zi = 2.0 * zr_abs * zi_abs + cy;

        zr = new_zr;
        zi = new_zi;
        iter++;

        if (zr * zr + zi * zi > 4.0)
        {
            break;
        }
    }

    iter_out = iter;
    bool escaped = (zr * zr + zi * zi) > 4.0;
    escaped_out = escaped;

    if (!escaped)
    {
        return 0xFF000000;
    }

    int bin = (iter * PALETTE_SIZE) / max_iteraciones;
    if (bin >= PALETTE_SIZE)
    {
        bin = PALETTE_SIZE - 1;
    }
    return color_ramp[bin];
}

void burning_ship_mpi(double x_min, double y_min, double x_max, double y_max,
                       uint32_t width, uint32_t height,
                       uint32_t row_start, uint32_t row_end,
                       uint32_t *pixel_buffer,
                       long local_hist[BURNING_SHIP_NBINS])
{
    double dx = (x_max - x_min) / width;
    double dy = (y_max - y_min) / height;

    std::memset(local_hist, 0, BURNING_SHIP_NBINS * sizeof(long));

    for (uint32_t j = row_start; j < row_end; j++)
    {
        for (uint32_t i = 0; i < width; i++)
        {
            double x = x_min + i * dx;
            double y = y_min + j * dy;

            int iter = 0;
            bool escaped = false;
            uint32_t color = burning_ship_pixel(x, y, iter, escaped);

            pixel_buffer[(j - row_start) * width + i] = color;

            if (escaped)
            {
                int bin = (iter * BURNING_SHIP_NBINS) / max_iteraciones;
                if (bin >= BURNING_SHIP_NBINS)
                {
                    bin = BURNING_SHIP_NBINS - 1;
                }
                local_hist[bin]++;
            }
        }
    }
}
