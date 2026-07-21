#include "vector_matriz.h"
#include <immintrin.h> 

void mat_serial(const float* mat, const float* vec, float* resultado, int n ){

    int dimensionM = n*n;
    for (size_t i = 0; i < dimensionM; i++)
    {
        float sum = 0;
        for (size_t j = 0; j < n;j++)
        {
            sum += mat[i * n * j] * vec[i];
        }
        resultado[i] = sum;
    }
    

}

void mat_vec_serial(const double* mat, const double* vec, double* res, int M, int N) {
    for (int i = 0; i < M; ++i) {
        double sum = 0.0;
        for (int j = 0; j < N; ++j) {
            sum += mat[i * N + j] * vec[j];
        }
        res[i] = sum;
    }
}

#include <immintrin.h>

void mat_vec_simd(const double* mat, const double* vec, double* res, int M, int N) {
    for (int i = 0; i < M; ++i) {
        __m256d sum_vec = _mm256_setzero_pd(); // acumulador de 4 doubles
        int j = 0;
        // procesar de a 4 elementos
        for (; j + 3 < N; j += 4) {
            __m256d mat_vals = _mm256_loadu_pd(&mat[i * N + j]);
            __m256d vec_vals = _mm256_loadu_pd(&vec[j]);
            __m256d prod = _mm256_mul_pd(mat_vals, vec_vals);
            sum_vec = _mm256_add_pd(sum_vec, prod);
        }
        //reducir los 4 doubles a uno solo
        __m128d low = _mm256_castpd256_pd128(sum_vec);          
        __m128d high = _mm256_extractf128_pd(sum_vec, 1);        
        __m128d sum128 = _mm_add_pd(low, high);                  
        // sumar los dos elementos restantes
        __m128d sum_final = _mm_hadd_pd(sum128, sum128);       
        double total = _mm_cvtsd_f64(sum_final);                


        for (; j < N; ++j) {
            total += mat[i * N + j] * vec[j];
        }
        res[i] = total;
    }
}

#include <omp.h>

void mat_vec_openmp(const double* mat, const double* vec, double* res, int M, int N) {
    #pragma omp parallel for
    for (int i = 0; i < M; ++i) {
        double sum = 0.0;
        for (int j = 0; j < N; ++j) {
            sum += mat[i * N + j] * vec[j];
        }
        res[i] = sum;
    }
}