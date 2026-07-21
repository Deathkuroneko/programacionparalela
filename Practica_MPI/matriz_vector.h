#ifndef MATRIZ_VEC_H
#define MATRIZ_VEC_H

void mat_vec_mpi(const double* mat, const double* vec, double* res, int M, int N);
void mat_vec_mpi_padding(const double* mat, const double* vec, double* res, int M, int N);


#endif