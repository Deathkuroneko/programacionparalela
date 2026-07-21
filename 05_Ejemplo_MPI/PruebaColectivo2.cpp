#include <mpi.h>
#include <vector>
#include <math.h>
#include <fmt/core.h>

using namespace std;

int MATRIX_DIM = 25;

void MultiplicarValores(vector<int>& A, vector<int>& B, vector<int>& X, int row, int col){
    for (int i = 0; i < row; i++)
    {
        int sumaTMP = 0;
        for (int j = 0; j < col; j++)
        {
            sumaTMP += A[i*col+j]*B[j];
        }
        X[i] = sumaTMP;
    }
    

}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int nprocs, rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    int padding = ceil((MATRIX_DIM * 1.0) / nprocs);
    int padding_rows = padding * nprocs;

    vector<int> A;
    vector<int> B(MATRIX_DIM, 1);;
    vector<int> X(padding_rows, 0);
    if (rank == 0)
    {
        A.resize(padding_rows * MATRIX_DIM, 0);
        

        for (int i = 0; i < (MATRIX_DIM); i++)
        {
            for (int j = 0; j < MATRIX_DIM; j++)
            {
                           A[i*MATRIX_DIM+j] = i;
            }
            
 
        }
    }
    MPI_Bcast(
        B.data(),
        MATRIX_DIM,
        MPI_INT,
        0,
        MPI_COMM_WORLD
    );
    vector<int> aTMP(padding*MATRIX_DIM);
    MPI_Scatter(
        A.data(),
        padding*MATRIX_DIM,
        MPI_INT,
        aTMP.data(),
        padding*MATRIX_DIM,
        MPI_INT,
        0,
        MPI_COMM_WORLD
    );
    vector<int> xTMP(padding,0);
    MultiplicarValores(aTMP,B,xTMP,padding,MATRIX_DIM);

    MPI_Gather(
        xTMP.data(),
        padding,
        MPI_INT,
        X.data(),
        padding,
        MPI_INT,
        0,
        MPI_COMM_WORLD
    );
    if (rank == 0)
    {
           for (int i = 0; i < MATRIX_DIM ; i++)
    {
        printf("\n %d", X[i]);
    }
    }
    

    


    MPI_Finalize();
    return 0;
}
