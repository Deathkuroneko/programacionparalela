#include <iostream>
#include <vector>
#include <algorithm>
#include <mpi.h>
#include <fmt/core.h>

int main(int argc, char** argv)
{
    // Inicializar MPI
    MPI_Init(&argc, &argv);

    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    const int TAMANO_GLOBAL = 12;

    // Verificar que el problema pueda repartirse uniformemente
    if (TAMANO_GLOBAL % nprocs != 0)
    {
        if (rank == 0)
        {
            fmt::print("Error: {} elementos no pueden dividirse entre {} procesos.\n",
                       TAMANO_GLOBAL, nprocs);
        }

        MPI_Finalize();
        return 0;
    }

    int elementos_reales = TAMANO_GLOBAL / nprocs;
    int tamano_local = elementos_reales + 2; // incluye fantasmas

    std::vector<double> datos_locales(tamano_local, 0.0);
    std::vector<double> resultado_local(elementos_reales, 0.0);

    //----------------------------------------------------------
    // El maestro reparte los datos
    //----------------------------------------------------------

    std::vector<double> datos_globales;

    if (rank == 0)
    {
        datos_globales = {
            1,2,3,4,
            5,6,7,8,
            9,10,11,12
        };

        fmt::print("[Maestro] Datos originales:\n");

        for (double x : datos_globales)
            fmt::print("{} ", x);

        fmt::print("\n-------------------------------------------\n");

        for (int i = 1; i < nprocs; i++)
        {
            MPI_Send(
                datos_globales.data() + i * elementos_reales,
                elementos_reales,
                MPI_DOUBLE,
                i,
                0,
                MPI_COMM_WORLD
            );
        }

        // El maestro copia directamente su bloque
        std::copy(
            datos_globales.begin(),
            datos_globales.begin() + elementos_reales,
            datos_locales.begin() + 1
        );
    }
    else
    {
        MPI_Recv(
            &datos_locales[1],
            elementos_reales,
            MPI_DOUBLE,
            0,
            0,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE
        );
    }

    //----------------------------------------------------------
    // Intercambio de fantasmas
    //----------------------------------------------------------

    // Enviar último elemento al vecino derecho
    if (rank < nprocs - 1)
    {
        MPI_Send(
            &datos_locales[elementos_reales],
            1,
            MPI_DOUBLE,
            rank + 1,
            1,
            MPI_COMM_WORLD
        );
    }

    // Recibir fantasma izquierdo
    if (rank > 0)
    {
        MPI_Recv(
            &datos_locales[0],
            1,
            MPI_DOUBLE,
            rank - 1,
            1,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE
        );
    }

    // Enviar primer elemento al vecino izquierdo
    if (rank > 0)
    {
        MPI_Send(
            &datos_locales[1],
            1,
            MPI_DOUBLE,
            rank - 1,
            2,
            MPI_COMM_WORLD
        );
    }

    // Recibir fantasma derecho
    if (rank < nprocs - 1)
    {
        MPI_Recv(
            &datos_locales[elementos_reales + 1],
            1,
            MPI_DOUBLE,
            rank + 1,
            2,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE
        );
    }

    //----------------------------------------------------------
    // Aplicar filtro de suavizado
    //----------------------------------------------------------

    for (int i = 1; i <= elementos_reales; i++)
    {
        resultado_local[i - 1] =
            (datos_locales[i - 1] +
             datos_locales[i] +
             datos_locales[i + 1]) / 3.0;
    }

    //----------------------------------------------------------
    // Reunir resultados
    //----------------------------------------------------------

    if (rank == 0)
    {
        std::vector<double> resultado_global(TAMANO_GLOBAL);

        std::copy(
            resultado_local.begin(),
            resultado_local.end(),
            resultado_global.begin()
        );

        for (int i = 1; i < nprocs; i++)
        {
            MPI_Recv(
                resultado_global.data() + i * elementos_reales,
                elementos_reales,
                MPI_DOUBLE,
                i,
                3,
                MPI_COMM_WORLD,
                MPI_STATUS_IGNORE
            );
        }

        fmt::print("[Maestro] Resultado filtrado:\n");

        for (double x : resultado_global)
            fmt::print("{:.2f} ", x);

        fmt::print("\n");
    }
    else
    {
        MPI_Send(
            resultado_local.data(),
            elementos_reales,
            MPI_DOUBLE,
            0,
            3,
            MPI_COMM_WORLD
        );
    }

    MPI_Finalize();
    return 0;
}