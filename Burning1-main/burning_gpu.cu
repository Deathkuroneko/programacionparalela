#include <cstdint>
#include <cmath>
#include <cuda_runtime.h>

const int PALETTE_SIZE = 16;

__constant__ // variable de solo lectura en la memoria de la GPU
unsigned int color_ramp[PALETTE_SIZE];

__device__ // funcion en la GPU, solo accesible desde la GPU
uint32_t acotado(double x, double y,
                  int max_iteraciones,
                  int *device_hist)
{


    int iter = 1;

    double zr = 0.0;
    double zi = 0.0;
    double cr = x;
    double ci = y;

    while (iter < max_iteraciones && (zr * zr + zi * zi) < 4.0)
    {
        double zr_abs = fabs(zr);
        double zi_abs = fabs(zi);

        double nuevoReal = zr_abs * zr_abs - zi_abs * zi_abs + cr;
        double nuevoImag = (2.0 * zr_abs * zi_abs) + ci;

        zr = nuevoReal;
        zi = nuevoImag;

        iter++;
    }

    if (iter < max_iteraciones)
    {
        int bin = iter * 16 / max_iteraciones;
        if (bin >= 16)
            bin = 15;

        atomicAdd(&device_hist[bin], 1);

        // nomras > 2
        int index = iter % PALETTE_SIZE;
        return color_ramp[index];
    }

    // los bits esta alreves en cuanto a los colores
    return 0xFF000000; // negro
}

__global__ // funcion en la GPU
void burning_kernel(
    int num_iteraciones,
    double x_min, double y_min, double x_max, double y_max,
    uint32_t width,
    uint32_t heigt,
    uint32_t *pixel_buffer,
    int *device_hist)
{
    double dx = (x_max - x_min) / width;
    double dy = (y_max - y_min) / heigt;
    int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index < width * heigt)
    {
        // z = x+yi = (x,y)
        int i = index % width;
        int j = index / width;
        double x = x_min + i * dx;
        double y = y_max - j * dy;

        // similar al var
        auto color = acotado(x, y, num_iteraciones, device_hist);

        // index j*w + i
        pixel_buffer[j * width + i] = color;
    }
}

void copiar_paleta(unsigned int *pallete_host)
{
    // forma de copiar variables del host a la gpu  cudaMemcpyToSymbol-- copiar la paleta desde la cpu a la gpu
    cudaMemcpyToSymbol(color_ramp, pallete_host, sizeof(unsigned int) * PALETTE_SIZE);
}

void burning_gpu(
    int num_iteraciones,
    double x_min, double y_min, double x_max, double y_max,
    uint32_t width,
    uint32_t heigt,
    uint32_t *pixel_buffer,
    int *device_hist)
{
    // limpiar histograma en el device antes de cada frame
    cudaMemset(device_hist, 0, 16 * sizeof(int));

    int threads_per_block = 1024;
    int blocks_per_grid = std::ceil((width * heigt) * 1.0 / threads_per_block);

    burning_kernel<<<blocks_per_grid, threads_per_block>>>(
        num_iteraciones,
        x_min, y_min, x_max, y_max,
        width, heigt,
        pixel_buffer, device_hist);
}