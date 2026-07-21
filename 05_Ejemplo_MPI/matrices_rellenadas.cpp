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

    int rows_per_rank = std::ceil(
        MATRIX_DIH * 1.0 / nprocs
    ); // filas por proceso, con padding

    int padded_rows = rows_per_rank * nprocs; // filas totales con padding

    if(rank == 0)
    {
        // CAMBIO 1: El tamaño original estaba al revés para soportar nprocs grandes. 
        // Cambiamos 'padded_rows * MATRIX_DIH' para asegurar que la memoria cubra todos los envíos.
        std::vector<double> A(
            padded_rows * MATRIX_DIH,
            0
        ); // matriz A con padding, inicializada a 0

        std::vector<double> B(
            MATRIX_DIH
        );

        std::vector<double> X(
            padded_rows,
            0
        ); // vector resultado con padding, inicializado a 0

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
            std::vector<int> info = {
                MATRIX_DIH,
                rows_per_rank
            }; // información de la matriz y filas asignadas a cada proceso

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
            ); // enviar bloque de filas de A a cada proceso con el padding incluido

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

        // CAMBIO 2: Para el Rank 0, calculamos cuántas de sus filas locales son reales.
        int filas_reales_rank0 = 0;
        for (int i = 0; i < rows_per_rank; i++) {
            if (0 * rows_per_rank + i < MATRIX_DIH) { // rank 0 * rows_per_rank
                filas_reales_rank0++;
            }
        }

        // Le pasamos las filas reales en lugar de todo el bloque con basura
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
    
        //quitar padding
        std::vector<double> resultado(
            X.begin(),
            X.begin()+MATRIX_DIH
        );

        imprimir_vector(resultado);
    }
    else
    {
        std::vector<int> info(2); // vector para recibir la información de la matriz y filas asignadas a cada proceso

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

        // CAMBIO 3: Aquí los ranks esclavos calculan dinámicamente qué filas son reales.
        // Multiplicamos nuestro rank por 'rows' para saber en qué fila de la matriz global empezamos.
        int inicio_fila_global = rank * rows;
        int filas_reales = 0;

        for (int i = 0; i < rows; i++) {
            if (inicio_fila_global + i < MATRIX_DIH) {
                filas_reales++;
            }
        }

        // Mandamos a multiplicar solo las filas reales calculadas, omitiendo la basura
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