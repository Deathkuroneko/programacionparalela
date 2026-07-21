#include <immintrin.h>
#include <iostream>
#include <vector>
#include <omp.h>
#include <cmath>
// Integrante: DYLAN ANTONIO LEMA CASA


// escalar serial
float productoEscalar(const std::vector<float>& x, const std::vector<float>& y)
{
    size_t n = x.size();

    __m256 suma = _mm256_setzero_ps();

    size_t i = 0;

    for (; i + 7 < n; i += 8)
    {
        __m256 vx = _mm256_loadu_ps(&x[i]);
        __m256 vy = _mm256_loadu_ps(&y[i]);

        __m256 multiplicar = _mm256_mul_ps(vx, vy);

        suma = _mm256_add_ps(suma, multiplicar);
    }

    float vector[8];
    _mm256_storeu_ps(vector, suma);

    float resultadoEscalar = 0.0f;

    for (int j = 0; j < 8; j++)
    {
        resultadoEscalar += vector[j];
    }

    for (; i < n; i++)
    {
        resultadoEscalar += x[i] * y[i];
    }

    return resultadoEscalar;
}

// openmp paralelo
float openmpEscalar(const std::vector<float>& a, const std::vector<float>& b)
{
    std::vector<float> result(a.size());
    float resultado = 0.0f;
    #pragma omp parallel
    {
        int thread_count = omp_get_num_threads();
        int thread_id = omp_get_thread_num();
        int delta = std::ceil(a.size() * 1.0 / thread_count);

        int start = thread_id * delta;
        int end = (thread_id + 1) * delta;
        end = std::min(end, (int)a.size());
        if(thread_id == thread_count-1){
            end = a.size();
        }
        for (int i = start; i < end; ++i) {
            result[i] = a[i] * b[i];
        }
        
        for (int i = start; i < end; ++i) {
            resultado += result[i];
        }
    }
    return resultado;

}

int main()
{
    std::vector<float> a = {1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,7,8,9}; // suman 90
    std::vector<float> b = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

    float serial = productoEscalar(a, b);

    float openOmp = openmpEscalar(a,b);

    std::cout << "Producto escalar AVX = " << serial << std::endl;
    std::cout << "Producto escalar Open MP = " << openOmp << std::endl;

    return 0;
}