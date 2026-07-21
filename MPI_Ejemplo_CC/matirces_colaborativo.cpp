#include <iostream>
#include <fmt/core.h>
#include <mpi.h>
#include <vector>
#include <cmath>

#define MATRIX_DIH 25

void imprimir_vector(const std::vector<double>& v) {
    for (size_t i = 0; i < v.size(); i++) {
        std::cout << v[i] << " ";
    }
    std::cout << std::endl;
}

void multiplicar_matriz_vector(
    const std::vector<double>& A,
    const std::vector<double>& b,
    std::vector<double>& X,
    int rows,
    int cols
)
{
    for (int i = 0; i < rows; i++) {
        double sum = 0.0;
        for (int j = 0; j < cols; j++) {
            sum += A[i * cols + j] * b[j];
        }
        X[i] = sum;
    }
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int nprocs;
    int rank;
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // redondeo hacia el mayor entero si tengo 3.xx -> 4, n=4
    int rows_per_rank = std::ceil(MATRIX_DIH * 1.0 / nprocs);
    // filas finales
    int padded_rows = rows_per_rank * nprocs;

    // vectores para cada proceso
    std::vector<double> B(MATRIX_DIH);
    std::vector<double> A_local(rows_per_rank * MATRIX_DIH);
    std::vector<double> X_local(rows_per_rank, 0.0);

    // dimensionar los vectore a y x, apra todos los procesos
    std::vector<double> A(padded_rows * MATRIX_DIH, 0.0);
    std::vector<double> X(padded_rows, 0.0);

    if (rank == 0) {
        // matriz A llenar con datos
        for (int i = 0; i < MATRIX_DIH; i++) {
            for (int j = 0; j < MATRIX_DIH; j++) {
                A[i * MATRIX_DIH + j] = i;
            }
        }

        // vector b
        for (int i = 0; i < MATRIX_DIH; i++) {
            B[i] = 1.0;
        }
    }

    // pasar el vector B con todos
    MPI_Bcast(B.data(), MATRIX_DIH, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // dividir y enviar la matriz A
    MPI_Scatter(
        A.data(), 
        rows_per_rank * MATRIX_DIH, MPI_DOUBLE, 
        A_local.data(), 
        rows_per_rank * MATRIX_DIH, MPI_DOUBLE, 
        0, 
        MPI_COMM_WORLD
    );

    // filtrado de filas reales
    int inicio_fila_global = rank * rows_per_rank;
    int filas_reales = 0;

    for (int i = 0; i < rows_per_rank; i++) {
        if (inicio_fila_global + i < MATRIX_DIH) {
            filas_reales++;
        }
    }

    // mutltiplicacion de la matriz y el vector de cada uno
    multiplicar_matriz_vector(A_local, B, X_local, filas_reales, MATRIX_DIH);

    // tengo los resultados y espero q se junte
    MPI_Gather(
        X_local.data(), 
        rows_per_rank, 
        MPI_DOUBLE, 
        X.data(), 
        rows_per_rank, 
        MPI_DOUBLE, 
        0, 
        MPI_COMM_WORLD
    );

    // ver resultado
    if (rank == 0) {
        std::cout << "\nRESULTADO FINAL\n";
        
        std::vector<double> resultado(X.begin(), X.begin() + MATRIX_DIH);
        imprimir_vector(resultado);
    }

    MPI_Finalize();
    return 0;
}