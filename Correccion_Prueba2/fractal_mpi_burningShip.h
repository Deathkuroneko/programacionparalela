#ifndef FRACTAL_SERIAL_H
#define FRACTAL_SERIAL_H

#include <cstdint>

#define BURNING_SHIP_NBINS 16

void burning_ship_mpi(double x_min, double y_min, double x_max, double y_max,
                       uint32_t width, uint32_t height,
                       uint32_t row_start, uint32_t row_end,
                       uint32_t *pixel_buffer,
                       long local_hist[BURNING_SHIP_NBINS]);

#endif
