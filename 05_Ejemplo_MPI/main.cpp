#include <iostream>
#include <fmt/core.h>
#include <mpi.h>

int main(int argc, char **argv)
{
 int nprocs, rank;   
    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    int version, subversion;
    MPI_Get_version(&version,&subversion);
    if (rank == 0)
    {
        fmt::println("MPI verasion {}.{}",version,subversion);
        fmt::println("MPI {}procesos",nprocs);
    }
    fmt::println("MPI rank{} {}procesos",rank,nprocs);

    MPI_Finalize();
    return 0;
}
