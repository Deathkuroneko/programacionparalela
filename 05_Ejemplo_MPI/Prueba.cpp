#include<mpi.h>
#include<iostream>

int main(int argc, char **argv)
{
    MPI_Init(&argc,&argv);
    int nprocs;
    int rank;

    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&nprocs);
   
    if (rank == 0)
    {
        int numero = 3;
        std::cout<<"Numero de rank"<<rank<<std::endl;
        for (int i = 1; i < nprocs; i++)
        {
            MPI_Send(
            &numero,
            1,
            MPI_INT,
            i,
            0,
            MPI_COMM_WORLD
        );
        }
        

    }
    else{
        int numRecv;
        MPI_Recv(
            &numRecv,
            1,
            MPI_INT,
            0,
            0,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE
        );
        std::cout<<"Numero de rank: "<<rank<<"Numero: "<<numRecv<<std::endl;

    }
    


    MPI_Finalize;
    return 0;
}
