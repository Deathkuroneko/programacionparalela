#include <fmt/core.h>
#include <mpi.h>
#include <iostream>
#include <vector>
#include <cmath>

using namespace std;

int VECSIZE = 19;

void multiplicarVector(vector<int>& a,
                       vector<int>& b,
                       vector<int>& x)
{
    for(int i=0;i<a.size();i++)
    {
        x[i]=a[i]*b[i];
    }
}

int main(int argc,char** argv)
{
    MPI_Init(&argc,&argv);

    int rank,nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&nprocs);

    int nDatos = ceil(VECSIZE*1.0/nprocs);
    int padding = nDatos*nprocs;

    vector<int> A;
    vector<int> B;
    vector<int> X;

    if(rank==0)
    {
        A.resize(padding,0);
        B.resize(padding,0);
        X.resize(padding,0);

        for(int i=0;i<VECSIZE;i++)
        {
            A[i]=i;
            B[i]=2;
        }
    }

    vector<int> aTMP(nDatos);
    vector<int> bTMP(nDatos);
    vector<int> xTMP(nDatos);

    // Repartir A
    MPI_Scatter(
        A.data(),
        nDatos,
        MPI_INT,
        aTMP.data(),
        nDatos,
        MPI_INT,
        0,
        MPI_COMM_WORLD
    );

    // Repartir B
    MPI_Scatter(
        B.data(),
        nDatos,
        MPI_INT,
        bTMP.data(),
        nDatos,
        MPI_INT,
        0,
        MPI_COMM_WORLD
    );

    // Procesar
    multiplicarVector(aTMP,bTMP,xTMP);

    // Reunir resultados
    MPI_Gather(
        xTMP.data(),
        nDatos,
        MPI_INT,
        X.data(),
        nDatos,
        MPI_INT,
        0,
        MPI_COMM_WORLD
    );
    
    if(rank==0)
    {
        for(int i=0;i<VECSIZE;i++)
        {
            printf("%d\n",X[i]);
        }
    }

    MPI_Finalize();
    return 0;
}