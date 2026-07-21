#include <mpi.h>
#include <fmt/core.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

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
void contarReperidos(vector<int> &datos, vector<int> &frecuencia)
{
    for (int i = 0; i < datos.size(); i++)
    {
        int dato = datos[i];
        frecuencia[dato]++;
    }
}

void sumaParcial(vector<int> &datos, int &suma)
{
    for (int i = 0; i < datos.size(); i++)
    {
        suma += datos[i];
    }
}
void maximoMinimo(vector<int> &datos, vector<int>& min_max)
{
        min_max[0] = datos[0];
        min_max[1] = datos[0];
    for (int i = 0; i < datos.size(); i++)
    {


        if (datos[i] < min_max[0])
        {
            min_max[0] = datos[i];
        }
        if (datos[i] > min_max[1])
        {
            min_max[1] = datos[i];
        }
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
        vector<int> datos = read_file();

        int sizeDatos = datos.size();
        int nDatos = ceil((sizeDatos * 1.0) / nprocs);
        for (int i = 0; i < nprocs; i++)
        {
            int datosEnvio = nDatos;
            if (i == (nprocs - 1))
            {
                datosEnvio = sizeDatos - (nDatos * (nprocs - 1));
            }
            vector<int> datosProc = {datosEnvio};
            MPI_Send(
                datosProc.data(),
                1,
                MPI_INT,
                i,
                0,
                MPI_COMM_WORLD);

            MPI_Send(
                datos.data() + i * nDatos,
                datosEnvio,
                MPI_INT,
                i,
                0,
                MPI_COMM_WORLD);
        }
        // HACER LA PARTE DE 0
        vector<int> datosTMP(datos.begin(), datos.begin() + nDatos);
        vector<int> frecuencia(101, 0);
        vector<int> frecuenciaTMP(101);
        contarReperidos(datosTMP, frecuencia);
        // recibir partes
        for (int i = 1; i < nprocs; i++)
        {
            MPI_Recv(
                frecuenciaTMP.data(),
                101,
                MPI_INT,
                i,
                0,
                MPI_COMM_WORLD,
                MPI_STATUS_IGNORE);
            for (int i = 0; i < frecuenciaTMP.size(); i++)
            {
                frecuencia[i] += frecuenciaTMP[i];
            }
        }
        printf("+-------+--------+");
        printf("| Valor | Conteo | ");
        printf("+-------+--------+");
        for (int i = 0; i < frecuencia.size(); i++)
        {
            printf("\n| %d | %d |", i, frecuencia[i]);
        }
        // Promedio
        int sumaTmp = 0;
        int suma = 0;
        float promedio = 0.0;
        sumaParcial(datosTMP, suma);
        for (int i = 1; i < nprocs; i++)
        {

            MPI_Recv(
                &sumaTmp,
                1,
                MPI_INT,
                i,
                0,
                MPI_COMM_WORLD,
                MPI_STATUS_IGNORE);
            suma += sumaTmp;
        }
        promedio = ((suma * 1.0) / sizeDatos);

        printf("\n Promedio de los datos: %f", promedio);
        //minimo maximo
        vector<int> min_maxTMP(2);
        vector<int> min_max(2);
        int minimo = 0;
        int maximo = 0;
        maximoMinimo(datosTMP,min_maxTMP);
        
        minimo = min_maxTMP[0];
        maximo = min_maxTMP[1];
        for (int i = 1; i < nprocs; i++)
        {
                MPI_Recv(
                min_max.data(),
                2,
                MPI_INT,
                i,
                0,
                MPI_COMM_WORLD,
                MPI_STATUS_IGNORE);
                if (min_max[0]<minimo)
                {
                    minimo = min_max[0];
                }
                if (min_max[1]>maximo)
                {
                    maximo = min_max[1];
                }
                
                
        }
        printf("\n Minimo: %d",minimo);
        printf("\n Maximo: %d",maximo);
        
        
        
    }
    else
    {
        int nDatos;
        vector<int> datTemp(1);
        MPI_Recv(
            datTemp.data(),
            1,
            MPI_INT,
            0,
            0,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE);
        nDatos = datTemp[0];
        //
        vector<int> datosTMP(nDatos);
        MPI_Recv(
            datosTMP.data(),
            nDatos,
            MPI_INT,
            0,
            0,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE);
        vector<int> frecuencia(101, 0);
        contarReperidos(datosTMP, frecuencia);
        // devolver datos
        MPI_Send(
            frecuencia.data(),
            101,
            MPI_INT,
            0,
            0,
            MPI_COMM_WORLD);
        // Parte para promedio
        int sumaTmp = 0;
        sumaParcial(datosTMP, sumaTmp);
        MPI_Send(
            &sumaTmp,
            1,
            MPI_INT,
            0,
            0,
            MPI_COMM_WORLD);
        // minimo y maximo
        vector<int> min_max(2);
        maximoMinimo(datosTMP, min_max);
        MPI_Send(
            min_max.data(),
            2,
            MPI_INT,
            0,
            0,
            MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;
}
