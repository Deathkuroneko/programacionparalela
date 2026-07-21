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

    // Todos los procesos calculan la distribución con padding por igual
    int rows_per_rank = std::ceil(MATRIX_DIH * 1.0 / nprocs);
    int padded_rows = rows_per_rank * nprocs;

    // Estructuras locales obligatorias para cada proceso
    std::vector<double> B(MATRIX_DIH);
    std::vector<double> A_local(rows_per_rank * MATRIX_DIH);
    std::vector<double> X_local(rows_per_rank, 0.0);

    // SOLUCIÓN: Dimensionar los contenedores globales en TODOS los procesos
    // Esto evita pasar punteros nulos inválidos a MPI_Scatter y MPI_Gather
    std::vector<double> A(padded_rows * MATRIX_DIH, 0.0);
    std::vector<double> X(padded_rows, 0.0);

    if (rank == 0) {
        // Inicializar la matriz original solo en el proceso raíz
        for (int i = 0; i < MATRIX_DIH; i++) {
            for (int j = 0; j < MATRIX_DIH; j++) {
                A[i * MATRIX_DIH + j] = i;
            }
        }

        // Inicializar el vector B en el proceso raíz
        for (int i = 0; i < MATRIX_DIH; i++) {
            B[i] = 1.0;
        }
    }

    // =================================================================
    // COMUNICACIÓN COLECTIVA SEGURA
    // =================================================================

    // 1. Compartir el vector B con todos
    MPI_Bcast(B.data(), MATRIX_DIH, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // 2. Dividir y enviar la matriz A de forma simétrica (Pasando parámetros seguros)
    MPI_Scatter(
        A.data(), rows_per_rank * MATRIX_DIH, MPI_DOUBLE, 
        A_local.data(), rows_per_rank * MATRIX_DIH, MPI_DOUBLE, 
        0, MPI_COMM_WORLD
    );

    // 3. Lógica de filtrado de filas reales
    int inicio_fila_global = rank * rows_per_rank;
    int filas_reales = 0;

    for (int i = 0; i < rows_per_rank; i++) {
        if (inicio_fila_global + i < MATRIX_DIH) {
            filas_reales++;
        }
    }

    // Calcular la multiplicación local respetando los límites reales
    multiplicar_matriz_vector(A_local, B, X_local, filas_reales, MATRIX_DIH);

    // 4. Reunir las soluciones parciales en el vector global X de forma segura
    MPI_Gather(
        X_local.data(), rows_per_rank, MPI_DOUBLE, 
        X.data(), rows_per_rank, MPI_DOUBLE, 
        0, MPI_COMM_WORLD
    );

    // El proceso raíz extrae e imprime el resultado final sin el padding
    if (rank == 0) {
        std::cout << "\nRESULTADO FINAL\n";
        
        std::vector<double> resultado(X.begin(), X.begin() + MATRIX_DIH);
        imprimir_vector(resultado);
    }

    MPI_Finalize();
    return 0;
}