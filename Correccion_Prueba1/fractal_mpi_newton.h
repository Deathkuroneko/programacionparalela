#ifndef FRACTAL_SERIAL_H
#define FRACTAL_SERIAL_H

#include <cstdint>

#define NEWTON_NUM_ROOTS 3
#define NEWTON_EPS 1e-4

void newton_mpi(double x_min, double y_min, double x_max, double y_max,
                 uint32_t width, uint32_t height,
                 uint32_t row_start, uint32_t row_end,
                 uint32_t *pixel_buffer,
                 double &compute_ms, long &total_iters);

#endif
