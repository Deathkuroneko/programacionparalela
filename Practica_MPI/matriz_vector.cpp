#include <vector>
#include <mpi.h>
#include "matriz_vector.h"

void mat_vec_mpi(const double* mat, const double* vec, double* res, int M, int N) {
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Obtener ID del proceso
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Obtener número total de procesos

    // 1. Calcular cuántas filas le tocan a cada proceso
    int rows_per_proc = M / size;
    int extra_rows = M % size;

    // Cada proceso sabe cuántas filas va a procesar
    int local_M = rows_per_proc + (rank < extra_rows ? 1 : 0);

    // 2. Preparar buffers para la recepción de datos
    std::vector<double> local_mat(local_M * N);
    std::vector<double> local_res(local_M);

    // 3. El proceso 0 distribuye la matriz (MPI_Scatter)
    // Necesitamos un vector con el número de elementos que recibe cada proceso
    std::vector<int> send_counts(size);
    std::vector<int> displs(size);
    if (rank == 0) {
        for (int i = 0; i < size; ++i) {
            int rows_for_i = rows_per_proc + (i < extra_rows ? 1 : 0);
            send_counts[i] = rows_for_i * N;
            displs[i] = (i == 0) ? 0 : displs[i-1] + send_counts[i-1];
        }
    }

    // El vector se difunde a todos (MPI_Bcast)
    // Cada proceso necesita una copia local del vector completo
    std::vector<double> local_vec(N);
    if (rank == 0) {
        // El proceso 0 ya tiene el vector, solo lo copiamos
        std::copy(vec, vec + N, local_vec.begin());
    }
    MPI_Bcast(local_vec.data(), N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Distribuir la matriz (Scatterv para manejar tamaños variables)
    MPI_Scatterv(mat, send_counts.data(), displs.data(), MPI_DOUBLE,
                local_mat.data(), local_M * N, MPI_DOUBLE,
                0, MPI_COMM_WORLD);

    // 4. Cálculo local: multiplicar el bloque de filas por el vector
    for (int i = 0; i < local_M; ++i) {
        double sum = 0.0;
        for (int j = 0; j < N; ++j) {
            sum += local_mat[i * N + j] * local_vec[j];
        }
        local_res[i] = sum;
    }

    // 5. Reunir los resultados parciales en el proceso 0 (MPI_Gatherv)
    std::vector<int> recv_counts(size);
    std::vector<int> rdispls(size);
    if (rank == 0) {
        for (int i = 0; i < size; ++i) {
            int rows_for_i = rows_per_proc + (i < extra_rows ? 1 : 0);
            recv_counts[i] = rows_for_i;
            rdispls[i] = (i == 0) ? 0 : rdispls[i-1] + recv_counts[i-1];
        }
    }

    MPI_Gatherv(local_res.data(), local_M, MPI_DOUBLE,
                res, recv_counts.data(), rdispls.data(), MPI_DOUBLE,
                0, MPI_COMM_WORLD);
}

void mat_vec_mpi_simple(const double* mat, const double* vec, double* res, int M, int N) {
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Suponemos que M es divisible por size
    int local_M = M / size;

    std::vector<double> local_mat(local_M * N);
    std::vector<double> local_vec(N);
    std::vector<double> local_res(local_M);

    // El proceso 0 difunde el vector
    if (rank == 0) {
        std::copy(vec, vec + N, local_vec.begin());
    }
    MPI_Bcast(local_vec.data(), N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Distribuir la matriz (Scatter normal)
    MPI_Scatter(mat, local_M * N, MPI_DOUBLE,
                local_mat.data(), local_M * N, MPI_DOUBLE,
                0, MPI_COMM_WORLD);

    // Cálculo local
    for (int i = 0; i < local_M; ++i) {
        double sum = 0.0;
        for (int j = 0; j < N; ++j) {
            sum += local_mat[i * N + j] * local_vec[j];
        }
        local_res[i] = sum;
    }

    // Reunir resultados (Gather normal)
    MPI_Gather(local_res.data(), local_M, MPI_DOUBLE,
            res, local_M, MPI_DOUBLE,
            0, MPI_COMM_WORLD);
}

void mat_vec_mpi_padding(const double* mat, const double* vec, double* res, int M, int N) {
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // 1. Calcular el número de filas por proceso (redondeando hacia arriba)
    int local_M = (M + size - 1) / size;

    // 2. Calcular el nuevo tamaño total de la matriz (con padding)
    int M_padded = local_M * size;

    // 3. Crear una matriz con padding (solo en el proceso 0)
    std::vector<double> mat_padded;
    if (rank == 0) {
        mat_padded.resize(M_padded * N, 0.0); // Inicializar con ceros
        // Copiar las filas reales de la matriz original
        for (int i = 0; i < M; ++i) {
            std::copy(mat + i * N, mat + (i + 1) * N, mat_padded.begin() + i * N);
        }
        // Las filas extra (desde M hasta M_padded-1) ya están a cero
    }

    // 4. Vector local y resultado local
    std::vector<double> local_vec(N);
    std::vector<double> local_res(local_M);

    // 5. Difundir el vector
    if (rank == 0) {
        std::copy(vec, vec + N, local_vec.begin());
    }
    MPI_Bcast(local_vec.data(), N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // 6. Repartir la matriz (Scatter normal)
    // Cada proceso recibe local_M * N elementos
    std::vector<double> local_mat(local_M * N);
    MPI_Scatter(mat_padded.data(), local_M * N, MPI_DOUBLE,
                local_mat.data(), local_M * N, MPI_DOUBLE,
                0, MPI_COMM_WORLD);

    // 7. Cálculo local (todas las filas, incluyendo las de padding)
    for (int i = 0; i < local_M; ++i) {
        double sum = 0.0;
        for (int j = 0; j < N; ++j) {
            sum += local_mat[i * N + j] * local_vec[j];
        }
        local_res[i] = sum;
    }

    // 8. Reunir todos los resultados (Gather normal)
    std::vector<double> res_padded;
    if (rank == 0) {
        res_padded.resize(M_padded);
    }
    MPI_Gather(local_res.data(), local_M, MPI_DOUBLE,
            res_padded.data(), local_M, MPI_DOUBLE,
            0, MPI_COMM_WORLD);

    // 9. Copiar solo las primeras M filas al resultado final
    if (rank == 0) {
        std::copy(res_padded.begin(), res_padded.begin() + M, res);
    }
}