#ifndef FRACTAL_SIMD_H

#define FRACTAL_SIMD_H

// seguros: PRAGMA ONCE

#include <cstdint>

void julia_serial_simd(double x_min, double y_min, double x_max, double y_max, uint32_t width, uint32_t heigt, uint32_t* pixel_buffer);


#endif