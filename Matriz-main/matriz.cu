#include <cstdio>
#include <cstdlib>
#include <cuda_runtime.h>

#define M 1024

__global__ void matVecMulKernel(const float *A, const float *B, float *C, int m)
{
    int row = blockIdx.x * blockDim.x + threadIdx.x;

    if (row < m)
    {
        float suma = 0.0f;

        for (int j = 0; j < m; j++)
            suma += A[row * m + j] * B[j];

        C[row] = suma;
    }
}

void checkCuda(cudaError_t err, const char *msg)
{
    if (err != cudaSuccess)
    {
        fprintf(stderr, "Error CUDA (%s): %s\n", msg, cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
}

void inicializar(float *A, float *B)
{
    for (int i = 0; i < M; i++)
    {
        B[i] = 1.0f;

        for (int j = 0; j < M; j++)
            A[i * M + j] = 1.0f;
    }
}

void multiplicacionMatrizVector()
{
    size_t sizeMat = (size_t)M * M * sizeof(float);
    size_t sizeVec = (size_t)M * sizeof(float);

    float *h_A = (float *)malloc(sizeMat);
    float *h_B = (float *)malloc(sizeVec);
    float *h_C = (float *)malloc(sizeVec);

    inicializar(h_A, h_B);

    float *d_A;
    float *d_B;
    float *d_C;

    checkCuda(cudaMalloc(&d_A, sizeMat), "cudaMalloc A");
    checkCuda(cudaMalloc(&d_B, sizeVec), "cudaMalloc B");
    checkCuda(cudaMalloc(&d_C, sizeVec), "cudaMalloc C");

    checkCuda(cudaMemcpy(d_A, h_A, sizeMat, cudaMemcpyHostToDevice), "Copiar A");
    checkCuda(cudaMemcpy(d_B, h_B, sizeVec, cudaMemcpyHostToDevice), "Copiar B");

    int threadsPerBlock = 256;
    int blocksPerGrid = (M + threadsPerBlock - 1) / threadsPerBlock;

    matVecMulKernel<<<blocksPerGrid, threadsPerBlock>>>(d_A, d_B, d_C, M);

    checkCuda(cudaGetLastError(), "Lanzamiento Kernel");
    checkCuda(cudaDeviceSynchronize(), "Sincronizacion");

    checkCuda(cudaMemcpy(h_C, d_C, sizeVec, cudaMemcpyDeviceToHost), "Copiar C");

    printf("Resultado:\n");
    printf("C[0] = %.2f\n", h_C[0]);
    printf("C[%d] = %.2f\n", M - 1, h_C[M - 1]);

    cudaFree(d_A);
    cudaFree(d_B);
    cudaFree(d_C);

    free(h_A);
    free(h_B);
    free(h_C);
}

int main()
{
    multiplicacionMatrizVector();

    return 0;
}