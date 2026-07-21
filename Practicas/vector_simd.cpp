#include "vector_simd.h"
#include <immintrin.h> 

void serial_simd_int(const int* vectorA, const int* vectorB,int* resultado, int n){

    int b  = 0;
    for (size_t i = 0; i+7 < n; i +=8)
    {
        __m256i a_vec = _mm256_loadu_si256((const __m256i*)&vectorA[i]);
        __m256i b_vec = _mm256_loadu_si256((const __m256i*)&vectorB[i]);
        __m256i c_vec = _mm256_add_epi32(a_vec, b_vec);
        _mm256_storeu_si256((__m256i*)&resultado[i], c_vec);
        b = i;
    }
    
    for (size_t i = b ; i < n; i++)
    {
        resultado[i] = vectorA[i] + vectorB[i];
    }
    
}


void serial_simd_int_sin_asignar(int* resultado, int n){

    const __m256i vecA = _mm256_set1_epi32(1);
    const __m256i vecB = _mm256_set1_epi32(2);

    int b = 0;
    for (size_t i = 0; i+7 < n; i += 8)
    {
        __m256i vecC = _mm256_add_epi32(vecA,vecB);
        _mm256_storeu_si256((__m256i*)&resultado[i], vecC);
        b=i;
    }
    
    for (size_t i = b; i < n; i++)
    {
        //rellenado valores faltantes con lo q era la posible suma
        resultado[i] = 3;
    }
    
}

void serial_simd_double_resta(const double* vectorA, const double* vectorB,double* resultado, int n){

    int b = 0;
    for (size_t i = 0; i+3 < n; i += 4)
    {
        __m256d vecA = _mm256_loadu_pd(&vectorA[i]);
        __m256d vecB = _mm256_loadu_pd(&vectorB[i]);
        __m256d vecC = _mm256_sub_pd(vecA,vecB);
        _mm256_storeu_pd(&resultado[i], vecC);
        b = i;
    }

    for (size_t i = b; i < n; i++)
    {
        resultado[i] = vectorA[i] - vectorB[i];
    }
    
}

void serial_simd_float_resta(const float* vectorA, const float* vectorB,float* resultado, int n){

    int b = 0;
    for (size_t i = 0; i+7 < n; i += 8)
    {
        __m256 vecA = _mm256_loadu_ps(&vectorA[i]);
        __m256 vecB = _mm256_loadu_ps(&vectorB[i]);
        __m256 vecC = _mm256_sub_ps(vecA,vecB);
        _mm256_storeu_ps(&resultado[i], vecC);
        b = i;
    }

    for (size_t i = b; i < n; i++)
    {
        resultado[i] = vectorA[i] - vectorB[i];
    }
    
}

void serial_simd_float_resta_sin_asignar(float* resultado, int n){

    const __m256 vecA = _mm256_set1_ps(1);
    const __m256 vecB = _mm256_set1_ps(2);

    int b = 0;
    for (size_t i = 0; i+7 < n; i += 8)
    {
        __m256 vecC = _mm256_sub_ps(vecA,vecB);
        _mm256_storeu_ps(&resultado[i], vecC);
        b = i;
    }

    for (size_t i = b; i < n; i++)
    {
        resultado[i] = 1;
    }
    
}

void serial_simd_double_resta_sin_asignar(double* resultado, int n){

    const __m256d vecA = _mm256_set1_pd(1);
    const __m256d vecB = _mm256_set1_pd(2);

    int b = 0;
    for (size_t i = 0; i+3 < n; i += 4)
    {
        __m256d vecC = _mm256_sub_pd(vecA,vecB);
        _mm256_storeu_pd(&resultado[i], vecC);
        b = i;
    }

    for (size_t i = b; i < n; i++)
    {
        resultado[i] = 1;
    }
    
}