#include <mpi.h>
#include <iostream>

int main(int argc, char *argv[])
{
    const int M = 32;

    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (M % size != 0)
    {
        if (rank == 0)
            std::cout << "La matriz debe ser divisible por el numero de procesos.\n";

        MPI_Finalize();
        return 0;
    }

    int filasLocales = M / size;

    double *A = nullptr;
    double *C = nullptr;

    double *B = new double[M];
    double *A_local = new double[filasLocales * M];
    double *C_local = new double[filasLocales];

    // Solo el proceso 0 crea e inicializa la matriz y el vector
    if (rank == 0)
    {
        A = new double[M * M];
        C = new double[M];

        for (int i = 0; i < M; i++)
        {
            B[i] = 1.0;

            for (int j = 0; j < M; j++)
            {
                A[i * M + j] = 1.0;
            }
        }
    }

    // Compartir el vector B con todos los procesos
    MPI_Bcast(B, M, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Repartir las filas de la matriz
    MPI_Scatter(
        A,
        filasLocales * M,
        MPI_DOUBLE,
        A_local,
        filasLocales * M,
        MPI_DOUBLE,
        0,
        MPI_COMM_WORLD);

    // Multiplicación matriz-vector
    for (int i = 0; i < filasLocales; i++)
    {
        C_local[i] = 0.0;

        for (int j = 0; j < M; j++)
        {
            C_local[i] += A_local[i * M + j] * B[j];
        }
    }

    // Reunir el resultado final
    MPI_Gather(
        C_local,
        filasLocales,
        MPI_DOUBLE,
        C,
        filasLocales,
        MPI_DOUBLE,
        0,
        MPI_COMM_WORLD);

    if (rank == 0)
    {
        std::cout << "Resultado del producto matriz-vector\n";
        std::cout << "C[0] = " << C[0] << std::endl;
        std::cout << "C[" << M - 1 << "] = " << C[M - 1] << std::endl;

        delete[] A;
        delete[] C;
    }

    delete[] B;
    delete[] A_local;
    delete[] C_local;

    MPI_Finalize();
    return 0;
}