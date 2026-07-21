#ifndef FRACTAL_SERIAL_H

#define FRACTAL_SERIAL_H

#include <cstdint>
#include <vector>


void burning_serial(double x_min, double y_min, double x_max, double y_max, 
    uint32_t width, uint32_t height,
    uint32_t *pixel_buffer,std::vector<int>& local_hist);

void burning_omp(double x_min, double y_min, double x_max, double y_max, 
    uint32_t width, uint32_t height,
    uint32_t *pixel_buffer,std::vector<int>& local_hist);
void burning_simd(double x_min, double y_min, double x_max, double y_max, 
    uint32_t width, uint32_t height,
    uint32_t *pixel_buffer,std::vector<int>& local_hist);

#endif