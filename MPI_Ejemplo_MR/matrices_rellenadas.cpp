#include <iostream>
#include <fmt/core.h>
#include <mpi.h>
#include <vector>
#include <cmath>

#define MATRIX_DIH 25

void imprimir_matriz(const std::vector<double>& A_local,
                    int rows,
                    int matrix_dim) {

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < matrix_dim; j++) {
            std::cout << A_local[i * matrix_dim + j] << " ";
        }
        std::cout << std::endl;
    }
}

void imprimir_vector(const std::vector<double>& v) {

    for (int i = 0; i < v.size(); i++) {
        std::cout << v[i] << " ";
    }

    std::cout << std::endl;
}

void multiplicar_matriz_vector(
    std::vector<double>& A,
    std::vector<double>& b,
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

int main(int argc, char **argv)
{
    MPI_Init(&argc,&argv);

    int nprocs;
    int rank;

    MPI_Comm_size(MPI_COMM_WORLD,&nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    // redonde al proximo entero 3.xx se sube al siguiente 4, n = 4
    int rows_per_rank = std::ceil(MATRIX_DIH * 1.0 / nprocs); 

    // filas totales
    int padded_rows = rows_per_rank * nprocs; 

    if(rank == 0)
    {

        std::vector<double> A(padded_rows * MATRIX_DIH,0); 

        std::vector<double> B(MATRIX_DIH);

        std::vector<double> X(padded_rows, 0);

        // relleno de la matriz A con valores
        for(int i=0;i<MATRIX_DIH;i++)
        {
            for(int j=0;j<MATRIX_DIH;j++)
            {
                A[i*MATRIX_DIH+j] = i;
            }
        }

        // vector b
        for(int i=0;i<MATRIX_DIH;i++)
        {
            B[i]=1;
        }
        
        //envio de datos a los procesos
        for(int i=0;i<nprocs;i++)
        {
            // info del vector, dimesion y filas por rank
            std::vector<int> info = {
                MATRIX_DIH,
                rows_per_rank
            }; 

            MPI_Send(
                info.data(),
                2,
                MPI_INT,
                i,
                0,
                MPI_COMM_WORLD
            );

            MPI_Send(
                A.data()+i*rows_per_rank*MATRIX_DIH,
                rows_per_rank*MATRIX_DIH,
                MPI_DOUBLE,
                i,
                0,
                MPI_COMM_WORLD
            );

            MPI_Send(
                B.data(),
                MATRIX_DIH,
                MPI_DOUBLE,
                i,
                0,
                MPI_COMM_WORLD
            );
        }

        std::vector<double> A_local(
            rows_per_rank*MATRIX_DIH
        );

        // extraemos la porcion d ematriz de rank 0
        for(int i=0;i<rows_per_rank;i++)
        {
            for(int j=0;j<MATRIX_DIH;j++)
            {
                A_local[i*MATRIX_DIH+j] =
                A[i*MATRIX_DIH+j];
            }
        }

        std::vector<double> X_local(
            rows_per_rank
        );

        // rank 0, calculamos filas locales q son reales
        int filas_reales_rank0 = 0;
        for (int i = 0; i < rows_per_rank; i++) {
            if (0 * rows_per_rank + i < MATRIX_DIH) {
                filas_reales_rank0++;
            }
        }

        // Le pasamos las filas reales
        multiplicar_matriz_vector(
            A_local,
            B,
            X_local,
            filas_reales_rank0,
            MATRIX_DIH
        );

        for(int i=0;i<rows_per_rank;i++)
        {
            X[i]=X_local[i];
        }

        //recibir resultados
        for(int i=1;i<nprocs;i++)
        {
            MPI_Recv(
                X.data()+i*rows_per_rank,
                rows_per_rank,
                MPI_DOUBLE,
                i,
                0,
                MPI_COMM_WORLD,
                MPI_STATUS_IGNORE
            );
        }

        std::cout<<"\nRESULTADO FINAL\n";
    
        // eliminacion de posiciones excedentes
        std::vector<double> resultado(
            X.begin(),
            X.begin()+MATRIX_DIH
        );

        imprimir_vector(resultado);
    }
    else
    {
        std::vector<int> info(2);

        MPI_Recv(
            info.data(),
            2,
            MPI_INT,
            0,
            0,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE
        );

        int matrix_dim = info[0];
        int rows = info[1];

        std::vector<double> A_local(
            rows*matrix_dim
        );

        std::vector<double> B(
            matrix_dim
        );

        MPI_Recv(
            A_local.data(),
            rows*matrix_dim,
            MPI_DOUBLE,
            0,
            0,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE
        );

        MPI_Recv(
            B.data(),
            matrix_dim,
            MPI_DOUBLE,
            0,
            0,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE
        );

        std::vector<double> X_local(
            rows
        );

        
        // filtramos las filas reales
        int inicio_fila_global = rank * rows;
        int filas_reales = 0;

        for (int i = 0; i < rows; i++) {
            if (inicio_fila_global + i < MATRIX_DIH) {
                filas_reales++;
            }
        }

        // multiplicamos solo las filas reales
        multiplicar_matriz_vector(
            A_local,
            B,
            X_local,
            filas_reales,
            matrix_dim
        );

        MPI_Send(
            X_local.data(),
            rows,
            MPI_DOUBLE,
            0,
            0,
            MPI_COMM_WORLD
        );
    }

    MPI_Finalize();

    return 0;
}