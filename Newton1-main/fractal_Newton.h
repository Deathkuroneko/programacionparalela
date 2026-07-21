#ifndef FRACTAL_NEWTON_H
#define FRACTAL_NEWTON_H

#include <cstdint>
#include <vector>

void newton_serial(double x_min, double y_min, double x_max, double y_max,
                   uint32_t width, uint32_t height,
                   uint32_t *pixel_buffer, long long &total_iters);
void newton_simd(double x_min, double y_min, double x_max, double y_max,
                   uint32_t width, uint32_t height,
                   uint32_t *pixel_buffer, long long &total_iters);
void newton_omp(double x_min, double y_min, double x_max, double y_max,
                   uint32_t width, uint32_t height,
                   uint32_t *pixel_buffer, long long &total_iters);

#endif