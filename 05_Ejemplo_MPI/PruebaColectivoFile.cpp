#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <fmt/core.h>
#include <mpi.h>
using namespace std;

std::vector<int> read_file()
{
    std::fstream fs("datos.txt", std::ios::in);
    std::string line;
    std::vector<int> ret;
    while (std::getline(fs, line))
    {
        ret.push_back(std::stoi(line));
    }
    fs.close();
    return ret;
}

void obtenerFrecuencia(vector<int> &datos, vector<int> &frecuencia)
{
    for (int i = 0; i < datos.size(); i++)
    {
        int dato = datos[i];
        frecuencia[dato]++;
    }
}

void sumarFrecuencias(vector<int>& datos, int& suma){
    for (int i = 0; i < datos.size(); i++)
    {
        suma += datos[i];
    }
    
}
void minimo_Maximo(vector<int>& datos, vector<int>& min_max){
    min_max[0] = datos[0];//minimo
    min_max[1] = datos[0];//maximo
    
    for (int i = 1; i < datos.size(); i++)
    {
        if (datos[i]<min_max[0])
        {
            min_max[0] = datos[i];
        }
        if (datos[i]>min_max[1])
        {
            min_max[1] = datos[i];
        }
    }
    
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int nprocs, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // carga de archivo
    int datosSize = 0;
    vector<int> datos;
    int datos_procesar = 0;
    int padding_datos;
    if (rank == 0)
    {
        datos = read_file();
        datosSize = datos.size();
        datos_procesar = ceil((datosSize * 1.0) / nprocs);
        padding_datos = datos_procesar * nprocs;
    }
    MPI_Bcast(&datos_procesar, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&padding_datos, 1, MPI_INT, 0, MPI_COMM_WORLD);
    vector<int> datosPadding(padding_datos, 0);
    if (rank == 0)
    {

        for (int i = 0; i < datosSize; i++)
        {
            datosPadding[i] = datos[i];
        }
    }
    vector<int> datosTMP(datos_procesar);
    MPI_Scatter(
        datosPadding.data(),
        datos_procesar,
        MPI_INT,
        datosTMP.data(),
        datos_procesar,
        MPI_INT,
        0,
        MPI_COMM_WORLD);
    vector<int> frecuenciaTMP(101, 0);
    vector<int> frecuenciaTotal(101, 0);

    obtenerFrecuencia(datosTMP, frecuenciaTMP);

    MPI_Reduce(
        frecuenciaTMP.data(),
        frecuenciaTotal.data(),
        101,
        MPI_INT,
        MPI_SUM,
        0,
        MPI_COMM_WORLD);
    if (rank == 0)
    {
        printf("+-------+--------+");
        printf("| Valor | Conteo | ");
        printf("+-------+--------+");
        for (int i = 0; i < frecuenciaTotal.size(); i++)
        {
            printf("\n| %d | %d |", i, frecuenciaTotal[i]);
        }
    }
    //PROMEDIO
    float promedio = 0;
    int sumaTMP = 0;
    int sumaTotal = 0;

    sumarFrecuencias(datosTMP,sumaTMP);
    MPI_Reduce(
        &sumaTMP,
        &sumaTotal,
        1,
        MPI_INT,
        MPI_SUM,
        0,
        MPI_COMM_WORLD
    );
    if (rank==0)
    {
        promedio = (sumaTotal*1.0)/datosSize;
        printf("\n El promedio es: %f", promedio);
    }
    // minimo_maximo
    vector<int> min_max(2,0);
    minimo_Maximo(datosTMP,min_max);
    int minimoGlobal = 0;
    int maximoGlobal = 0;

    MPI_Reduce(
        &min_max[0],
        &minimoGlobal,
        1,
        MPI_INT,
        MPI_MIN,
        0,
        MPI_COMM_WORLD
    );
    MPI_Reduce(
        &min_max[1],
        &maximoGlobal,
        1,
        MPI_INT,
        MPI_MAX,
        0,
        MPI_COMM_WORLD
    );
    if (rank == 0)
    {
        printf("\n MINIMO: %d",minimoGlobal);
        printf("\n MAXIMO: %d",maximoGlobal);
        
    }
    
    

    MPI_Finalize();
    return 0;
}
