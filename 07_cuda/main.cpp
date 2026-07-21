#include <iostream>
#include <fmt/core.h>
#include <cuda_runtime.h>
 
const int VECTOR_SIZE = 1020*1024; // 1MB
extern void sumaVector(float* a, float* b, float* c, int n);
 
int main(int argc, char const *argv[])
{
    int deviceId = 0;
    cudaSetDevice(deviceId);
 
    cudaDeviceProp deviceProp;
    cudaGetDeviceProperties(&deviceProp, deviceId);
    fmt::print("Device Name: {}\n", deviceProp.name);
    fmt::println("Total memory: {} MB", deviceProp.totalGlobalMem / (1024 * 1024));
    fmt::println("Multi-Processor Count: {}", deviceProp.multiProcessorCount);
    fmt::println("Max threads per multiprocessor: {}", deviceProp.maxThreadsPerMultiProcessor);
    fmt::println("Max threads per block: {}", deviceProp.maxThreadsPerBlock);
    fmt::println("Max grid size: {} x {} x {}", deviceProp.maxGridSize[0], deviceProp.maxGridSize[1], deviceProp.maxGridSize[2]);
 
    float* h_A = new float[VECTOR_SIZE];
    float* h_B = new float[VECTOR_SIZE];
    float* h_C = new float[VECTOR_SIZE];
 
    for (size_t i = 0; i < VECTOR_SIZE; ++i) {
        h_A[i] = 1.0f;
        h_B[i] = 2.0f;
        h_C[i] = 0.0f;
    }
 
 
    //inicializar divice
    float* d_A;
    float* d_B;
    float* d_C;
    size_t size_inbytes = VECTOR_SIZE *sizeof(float);
 
    cudaMalloc((void**)&d_A, size_inbytes);
    cudaMalloc((void**)&d_B, size_inbytes);
    cudaMalloc((void**)&d_C, size_inbytes);
 
    // copiar del host al divice
    cudaMemcpy(d_A, h_A, size_inbytes, cudaMemcpyHostToDevice);
    cudaMemcpy(d_B, h_B, size_inbytes, cudaMemcpyHostToDevice);
 
    //invocaR EL KERNEL
    sumaVector(d_A, d_B, d_C, VECTOR_SIZE);
 
    // COPIAR DEL DIVICE AL HOST
    cudaMemcpy(h_C, d_C, size_inbytes, cudaMemcpyDeviceToHost);
 
    // imprimri resultado
    for (size_t i = 0; i < VECTOR_SIZE; ++i) {
        fmt::print("C[{}] = {}\n", i, h_C[i]);
    }
 
    // liberar memoria
    cudaFree(d_A);
    cudaFree(d_B);
    cudaFree(d_C);
    delete[] h_A;  
    delete[] h_B;
    delete[] h_C;
 
    return 0;
}
 