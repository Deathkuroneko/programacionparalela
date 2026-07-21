#ifndef MATRIZ_VECTOR_H
#define MATRIZ_VECTOR_H

void mat_serial(const float* mat, const float* vec, float* resultado, int n );
void mat_vec_simd(const double* mat, const double* vec, double* res, int M, int N);
void mat_vec_openmp(const double* mat, const double* vec, double* res, int M, int N);
void mat_vec_serial(const double* mat, const double* vec, double* res, int M, int N);

#endif