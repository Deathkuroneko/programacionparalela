#ifndef ESCALA_H
#define ESCALA_H
#include <complex>
#include <cstdint>
void Gray_SIMD(uint32_t *pixel_buffer, int ancho, int alto);
void Gray_OPENMP(uint32_t *pixel_buffer, int ancho, int alto);
#endif
