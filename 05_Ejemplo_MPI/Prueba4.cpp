#include<mpi.h>
#include<fmt/core.h>
#include<math.h>
#include<vector>
using namespace std;

int MATRIX_DIM = 25;


void Multiplicar(vector<int>& A,vector<int>& B,vector<int>& X,int rows,int col){
    for (int i = 0; i < rows; i++)
    {
        int sumTMP = 0;
        for (int j = 0; j < col; j++)
        {
            sumTMP += A[i*col+j]*B[j];
        }
        X[i] = sumTMP;
    }
    

}

int main(int argc, char **argv)
{
    MPI_Init(&argc,&argv);
    int rank,nprocs;
    MPI_Comm_size(MPI_COMM_WORLD,&nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    if (rank==0)
    {
        int n_rows = ceil(MATRIX_DIM*1.0/nprocs);
        //int ultimo = (MATRIX_DIM-(nprocs-1*n_rows));

        int padded_rows = (n_rows*nprocs);

        vector<int> A (padded_rows*MATRIX_DIM,0);
        vector<int> B (MATRIX_DIM,1);
        vector<int> X (MATRIX_DIM,0);

        for (int i = 0; i < padded_rows; i++)
        {
            for (int j = 0; j < MATRIX_DIM; j++)
            {
                A[i*MATRIX_DIM+j] = i;
            }
        }

        //envio de datos informativos
        vector<int> info = {n_rows,MATRIX_DIM};
        for (int i = 0; i < nprocs; i++)
        {
             MPI_Send(
            info.data(),
            2,
            MPI_INT,
            i,
            0,
            MPI_COMM_WORLD
        );
        //envio de matrices y datos
             MPI_Send(
            A.data()+i*n_rows*MATRIX_DIM,
            n_rows*MATRIX_DIM,
            MPI_INT,
            i,
            0,
            MPI_COMM_WORLD
        );
             MPI_Send(
            B.data(),
            MATRIX_DIM,
            MPI_INT,
            i,
            0,
            MPI_COMM_WORLD
        );
        }

        vector<int> aTmp(A.begin(),A.begin()+MATRIX_DIM*n_rows);
        vector<int> xTmp(n_rows);

        Multiplicar(aTmp, B, xTmp, n_rows, MATRIX_DIM);

        for (int i = 0; i < xTmp.size(); i++)
        {
            X[i] = xTmp[i];
        }
        //recibir data de los otros ranks
        for (int i = 1; i < nprocs; i++)
        {
            MPI_Recv(
                X.data()+i*n_rows,
                n_rows,
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
        
        
       


    }else{
        vector<int> info (2);
        MPI_Recv(
            info.data(),
            2,
            MPI_INT,
            0,
            0,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE
        );
        int rows = info[0];
        int dim_Mat = info[1];
        
        vector<int> aTmp(rows*dim_Mat);
        vector<int> bTmp(dim_Mat);
        vector<int> xTmp(rows);
        //recepcion datos
        MPI_Recv(
            aTmp.data(),
            rows*dim_Mat,
            MPI_INT,
            0,
            0,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE
        );
        MPI_Recv(
            bTmp.data(),
            dim_Mat,
            MPI_INT,
            0,
            0,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE
        );
        //operar matrices
        Multiplicar(aTmp,bTmp,xTmp,rows,dim_Mat);
        //Regresar a rank 0
        MPI_Send(
            xTmp.data(),
            rows,
            MPI_INT,
            0,
            0,
            MPI_COMM_WORLD
        );


    }    



    MPI_Finalize();


    return 0;
}
