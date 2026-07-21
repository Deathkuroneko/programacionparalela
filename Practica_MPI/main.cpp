#include <vector>
#include <fmt/core.h>
#include <mpi.h>
#include "matriz_vector.h"
#include "matriz_vector.h"

void prueba_mat_vec_mpi() {
    const int M = 25, N = 25;
    std::vector<double> mat(M * N, 1.0); // Matriz de 25x25 llena de 1
    std::vector<double> vec(N, 2.0);     // Vector de 25 lleno de 2
    std::vector<double> res(M);

    // Llamar a la función MPI
    //mat_vec_mpi(mat.data(), vec.data(), res.data(), M, N);
    mat_vec_mpi_padding(mat.data(), vec.data(), res.data(), M, N);

    // Mostrar resultados solo desde el proceso 0
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == 0) {
        fmt::println("\n=== PRUEBA MATRIZ-VECTOR MPI ===");
        for (int i = 0; i < M; ++i) {
            fmt::print("res[{}] = {:.2f}  ", i, res[i]);
            if ((i + 1) % 5 == 0) fmt::println("");
        }
        fmt::println("");
    }
}

int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);
    prueba_mat_vec_mpi();
    MPI_Finalize();
    return 0;
}
