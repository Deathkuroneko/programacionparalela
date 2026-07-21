#include <iostream>
#include <fmt/core.h>
#include <cuda_runtime.h>

void mat_vec_cuda(const double* mat, const double* vec, double* res, int M, int N) {
    double *d_mat, *d_vec, *d_res;

    // 1. Reservar memoria en la GPU
    cudaMalloc((void**)&d_mat, M * N * sizeof(double));
    cudaMalloc((void**)&d_vec, N * sizeof(double));
    cudaMalloc((void**)&d_res, M * sizeof(double));

    // 2. Copiar datos del host a la GPU
    cudaMemcpy(d_mat, mat, M * N * sizeof(double), cudaMemcpyHostToDevice);
    cudaMemcpy(d_vec, vec, N * sizeof(double), cudaMemcpyHostToDevice);

    // 3. Configurar y lanzar el kernel
    // Usamos un bloque de 256 hilos y tantos bloques como sea necesario
    int threads_per_block = 256;
    int blocks = (M + threads_per_block - 1) / threads_per_block;

    mat_vec_kernel<<<blocks, threads_per_block>>>(d_mat, d_vec, d_res, M, N);

    // 4. Esperar a que termine el kernel y comprobar errores
    cudaDeviceSynchronize();
    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess) {
        std::cerr << "Error en el kernel CUDA: " << cudaGetErrorString(err) << std::endl;
        exit(-1);
    }

    // 5. Copiar el resultado de vuelta al host
    cudaMemcpy(res, d_res, M * sizeof(double), cudaMemcpyDeviceToHost);

    // 6. Liberar memoria en la GPU
    cudaFree(d_mat);
    cudaFree(d_vec);
    cudaFree(d_res);
}

void prueba_mat_vec_cuda() {
    const int M = 25, N = 25;
    std::vector<double> mat(M * N, 1.0); // Matriz de 25x25 llena de 1
    std::vector<double> vec(N, 2.0);     // Vector de 25 lleno de 2
    std::vector<double> res(M);

    // Llamar a la función CUDA
    mat_vec_cuda(mat.data(), vec.data(), res.data(), M, N);

    fmt::println("\n=== PRUEBA MATRIZ-VECTOR CUDA ===");
    for (int i = 0; i < M; ++i) {
        fmt::print("res[{}] = {:.2f}  ", i, res[i]);
        if ((i + 1) % 5 == 0) fmt::println("");
    }
    fmt::println("");
}

int main(int argc, char const *argv[]) {
    prueba_mat_vec_cuda();

    return 0;
}