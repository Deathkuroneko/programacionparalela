#include <fmt/core.h>
#include <SFML/Graphics.hpp>
#include <omp.h>
#include <cstdint>
#include <complex>
#include <iostream>
#include <vector>
#include "vector_serial.h"
#include "vector_simd.h"
#include "vector_matriz.h"

void init_data(std::vector<double>& mat, std::vector<double>& vec, int M, int N) {
    // Llenar matriz con valores (i+j) y vector con (j+1)
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            mat[i * N + j] = static_cast<double>(i + j);
        }
    }
    for (int j = 0; j < N; ++j) {
        vec[j] = static_cast<double>(j + 1);
    }
}

bool verificar_resultados(const double* res1, const double* res2, int M, double tolerancia = 1e-9) {
    for (int i = 0; i < M; ++i) {
        if (std::abs(res1[i] - res2[i]) > tolerancia) {
            fmt::println("Diferencia en posición {}: {} vs {}", i, res1[i], res2[i]);
            return false;
        }
    }
    return true;
}


void prueba_mat_vec() {
    const int M = 25;
    const int N = 25;

    std::vector<double> mat(M * N);
    std::vector<double> vec(N);
    std::vector<double> res_serial(M);
    std::vector<double> res_simd(M);
    std::vector<double> res_openmp(M);

    init_data(mat, vec, M, N);

    // Ejecutar las tres versiones
    mat_vec_serial(mat.data(), vec.data(), res_serial.data(), M, N);
    mat_vec_simd(mat.data(), vec.data(), res_simd.data(), M, N);
    mat_vec_openmp(mat.data(), vec.data(), res_openmp.data(), M, N);

    // Mostrar resultados
    fmt::println("\n=== RESULTADOS MATRIZ-VECTOR ({}x{}) ===", M, N);
    for (int i = 0; i < M; ++i) {
        fmt::print("y[{}] = {:.2f}", i, res_serial[i]);
        if ((i + 1) % 5 == 0) fmt::println("");
        else fmt::print("   ");
    }

    // Verificar que todos coinciden
    bool ok_simd = verificar_resultados(res_serial.data(), res_simd.data(), M);
    bool ok_openmp = verificar_resultados(res_serial.data(), res_openmp.data(), M);

    fmt::println("\nVerificación SIMD: {}", ok_simd ? "OK" : "FALLo");
    fmt::println("Verificación OpenMP: {}", ok_openmp ? "OK" : "FALLo");
}

//mpi


int main(int argc, char const *argv[])
{
    const int N = 25;
    std::vector<double> vectorA(N);
    std::vector<double> vectorB(N);
    std::vector<double> vectorC(N);

    for (size_t i = 0; i < N; i++)
    {
        vectorA [i] = 1;
    }
    for (size_t i = 0; i < N; i++)
    {
        vectorB [i] = 2;
    }    
    
    vector_serial(vectorA.data(), vectorB.data(), vectorC.data(), N);

    double total;
    for (size_t i = 0; i < N; i++)
    {
        total += vectorC[i];
    }
    
    fmt::println("Total: {}", total);

    for (size_t i = 0; i < N; i++)
    {
        fmt::print("posicion {}: {} - ", i  , vectorC[i]);
    }

    const int n2 = 30;
    std::vector<int> vecA(n2);
    std::vector<int> vecB(n2);
    std::vector<int> vecC(n2);

    for (size_t i = 0; i < n2; i++)
    {
        vecA [i] = 1;
    }
    for (size_t i = 0; i < n2; i++)
    {
        vecB [i] = 2;
    }  
    serial_simd_int(vecA.data(), vecB.data(),vecC.data(), n2);

    fmt::println("\nSimD");
    for (size_t i = 0; i < n2; i++)
    {
        fmt::print("Pos: {}, valor: {}",i, vecC[i]);
    }
    
    omp_set_num_threads(4);
    prueba_mat_vec();

    return 0;
}