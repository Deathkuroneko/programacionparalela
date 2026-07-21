#include <fmt/core.h>
#include <mpi.h>
#include <iostream>
#include <vector>
#include <math.h>
int VECSIZE = 19;

void multiplicarMatriz(std::vector<int> &a, std::vector<int> &b, std::vector<int> &x)
{
    for (int i = 0; i < a.size(); i++)
    {
        x[i] = a[i] * b[i];
    }
}

int main(int argc, char **argv)
{
    std::vector<int> A(VECSIZE, 0);
    std::vector<int> B(VECSIZE, 2);
    std::vector<int> X(VECSIZE, 0);

    for (int i = 0; i < A.size(); i++)
    {
        A[i] = i;
    }

    MPI_Init(&argc, &argv);
    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    if (rank == 0)
    {
        int padding = std::ceil(VECSIZE * 1.0 / nprocs);
        for (int i = 0; i < nprocs; i++)
        {
            int nDatos = padding;
            if (i == nprocs - 1)
            {
                nDatos = (VECSIZE - ((nprocs - 1) * padding));
            }

            MPI_Send(
                &nDatos,
                1,
                MPI_INT,
                i,
                0,
                MPI_COMM_WORLD);
            MPI_Send(
                A.data() + i * padding,
                nDatos,
                MPI_INT,
                i,
                0,
                MPI_COMM_WORLD);
            MPI_Send(
                B.data() + i * padding,
                nDatos,
                MPI_INT,
                i,
                0,
                MPI_COMM_WORLD);

            std::vector<int> aTMP(nDatos);
            std::vector<int> bTMP(nDatos);
            std::vector<int> xTMP(nDatos);

            for (int j = 0; j < nDatos; j++)
            {
                aTMP[j] = A[j];
                bTMP[j] = B[j];
            }

            multiplicarMatriz(aTMP, bTMP, xTMP);

            for (int j = 0; j < nDatos; j++)
            {
                X[j] = xTMP[j];
            }

            MPI_Recv(
                X.data() + (i * padding),
                nDatos,
                MPI_INT,
                i,
                0,
                MPI_COMM_WORLD,
                MPI_STATUS_IGNORE);
        }
        for (int i = 0; i < X.size(); i++)
        {
            printf("\n %d", X[i]);
        }
    }
    else
    {
        int nDatos;
        MPI_Recv(
            &nDatos,
            1,
            MPI_INT,
            0,
            0,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE);
        // printf("\n Procesar %d",nDatos);
        std::vector<int> aTMP(nDatos);
        std::vector<int> bTMP(nDatos);
        std::vector<int> xTMP(nDatos);

        MPI_Recv(
            aTMP.data(),
            nDatos,
            MPI_INT,
            0,
            0,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE);
        MPI_Recv(
            bTMP.data(),
            nDatos,
            MPI_INT,
            0,
            0,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE);

        multiplicarMatriz(aTMP, bTMP, xTMP);

        MPI_Send(
            xTMP.data(),
            nDatos,
            MPI_INT,
            0,
            0,
            MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;
}
