__global__ void mat_vec_kernel(const double* mat, const double* vec, double* res, int M, int N) {
    // Obtener el índice de la fila que procesa este hilo
    int row = blockIdx.x * blockDim.x + threadIdx.x;

    // Verificar que esté dentro del rango de filas
    if (row < M) {
        double sum = 0.0;
        // Recorrer las columnas de la fila
        for (int j = 0; j < N; ++j) {
            sum += mat[row * N + j] * vec[j];
        }
        res[row] = sum;
    }
}