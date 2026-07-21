#include <mpi.h>
#include <fmt/core.h>
#include <vector>
#include <math.h>
using namespace std;

int MATRIZ_DIM = 25;

void muktiplicarMatriz(vector<int>& a, vector<int>& b, vector<int>& x, int rows, int cols){
    
    for (int i = 0; i < rows; i++)
    {
        int sumaTmp = 0;
        for (int j = 0; j < cols; j++)
        {
            sumaTmp += a[i*cols+j] * b[j];
        }
        x[i] = sumaTmp;
    }
    
}


int main(int argc, char **argv)
{

    MPI_Init(&argc, &argv);
    int rank, nprocs;

    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == 0)
    {

        // AMPLIAMOS MATRIZ
        int rows_rank = std::ceil(MATRIZ_DIM * 1.0 / nprocs);

        int ampliarMatriz = rows_rank * nprocs;

        // INICIALIZAMOS MATRIZ
        std::vector<int> A(ampliarMatriz * MATRIZ_DIM, 0);
        std::vector<int> B(MATRIZ_DIM, 1);
        std::vector<int> X(MATRIZ_DIM, 0);
        for (int i = 0; i < MATRIZ_DIM; i++)
        {
            for (int j = 0; j < MATRIZ_DIM; j++)
            {
                A[i * MATRIZ_DIM + j] = i;
            }
        }
        std::vector<int> info = {rows_rank, MATRIZ_DIM};
        // envio
        for (int i = 0; i < nprocs; i++)
        {
            MPI_Send(
                info.data(),
                2,
                MPI_INT,
                i,
                0,
                MPI_COMM_WORLD);
            MPI_Send(
                A.data() + i * MATRIZ_DIM * rows_rank,
                rows_rank * MATRIZ_DIM,
                MPI_INT,
                i,
                0,
                MPI_COMM_WORLD);
            MPI_Send(
                B.data(),
                MATRIZ_DIM,
                MPI_INT,
                i,
                0,
                MPI_COMM_WORLD);
        }
        //PROCESAR PARTE DE RANK 0
        vector<int> aTmp(A.begin(),A.begin()+MATRIZ_DIM*rows_rank);

        vector<int> xTmp(rows_rank);
        muktiplicarMatriz(aTmp,B,xTmp,rows_rank,MATRIZ_DIM);
        for (int i = 0; i < xTmp.size(); i++)
        {
            X[i] = xTmp[i];
        }
        //recivir resultados
        for (int i = 1; i < nprocs; i++)
        {
            MPI_Recv(
                X.data()+i*rows_rank,
                rows_rank,
                MPI_INT,
                i,
                0,
                MPI_COMM_WORLD,
                MPI_STATUS_IGNORE
            );
        }
        
        for (int i = 0; i < X.size(); i++)
        {
            printf("\n %d",X[i]);
        }
        

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
            MPI_STATUS_IGNORE);
        int nRows = info[0];
        int dimMatrix = info[1];
        std::vector<int> Atmp(nRows * dimMatrix);
        std::vector<int> Btmp(dimMatrix);
        std::vector<int> Xtmp(nRows);
        // datos
        MPI_Recv(
            Atmp.data(),
            nRows * dimMatrix,
            MPI_INT,
            0,
            0,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE);
        MPI_Recv(
            Btmp.data(),
            dimMatrix,
            MPI_INT,
            0,
            0,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE);

        muktiplicarMatriz(Atmp,Btmp,Xtmp,nRows,dimMatrix);

        MPI_Send(
            Xtmp.data(),
            nRows,
            MPI_INT,
            0,
            0,
            MPI_COMM_WORLD
        );
        

    }

    MPI_Finalize();

    return 0;
}
