#include <immintrin.h>
#include <iostream>
#include <vector>
#include <omp.h>
#include <cmath>


//INTEGRANTES
//Byron Condolo
//Ruth Rosero
float productoEscalar(const std::vector<float>& x,
                      const std::vector<float>& y)
{
    size_t n = x.size();

    __m256 suma = _mm256_setzero_ps();

    size_t i = 0;

    for (i; i + 7 < n; i += 8)
    {
        __m256 xTmp = _mm256_loadu_ps(&x[i]);
        __m256 yTmp = _mm256_loadu_ps(&y[i]);

        __m256 mult = _mm256_mul_ps(xTmp, yTmp);

        suma = _mm256_add_ps(suma, mult);
    }

    float temp[8];
    _mm256_storeu_ps(temp, suma);

    float resultado = 0.0f;

    for (int j = 0; j < 8; j++)
    {
        resultado += temp[j];
    }

    for (i; i < n; i++)
    {
        resultado += x[i] * y[i];
    }

    return resultado;
}


float openmpEscalar(const std::vector<float>& a,
                    const std::vector<float>& b)
{
    
    int maxThreads = omp_get_max_threads();
    std::vector<float> sumaThread(maxThreads, 0.0f);
    #pragma omp parallel
    {
        int thread_count = omp_get_num_threads();
        int thread_id = omp_get_thread_num();

        int delta = std::ceil(a.size() * 1.0 / thread_count);

        int start = thread_id * delta;
        int end = (thread_id + 1) * delta;

        end = std::min(end, (int)a.size());

        if (thread_id == thread_count - 1)
        {
            end = a.size();
        }

        float suma = 0.0f;

        for (int i = start; i < end; ++i)
        {
            suma += a[i] * b[i];
        }

        sumaThread[thread_id] = suma;
    }

    float resultado = 0.0f;

    for (float v : sumaThread)
    {
        resultado += v;
    }

    return resultado;
}

int main()
{
    std::vector<float> a = {1,2,3,4,5,6,7,8,9};
    std::vector<float> b = {1,1,1,1,1,1,1,1,1};

    float r = productoEscalar(a, b);

    float rOMP = openmpEscalar(a,b);

    std::cout << "Producto escalar AVX = " << r << std::endl;
    std::cout << "Producto escalar Open MP = " << rOMP << std::endl;
    

    return 0;
}