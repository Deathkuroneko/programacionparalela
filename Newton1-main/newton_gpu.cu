#include <cstdint>
#include <cmath>
#include <cuda_runtime.h>

const int PALETTE_SIZE = 16;

__constant__ // variable de solo lectura en la memoria de la GPU
unsigned int color_ramp[PALETTE_SIZE];

__device__ // funcion en la GPU, solo accesible desde la GPU
uint32_t acotado(double x, double y,
                  int max_iteraciones,
                  unsigned long long *device_iters)
{

    int iter = 1;
    double zr = x;
    double zi = y;

    double raiz_r[3] = {1.0, -0.5, -0.5};
    double raiz_i[3] = {0.0, 0.8660254037844386, -0.8660254037844386}; // sqrt(3)/2

    while (iter < max_iteraciones)
    {
        // z^2
        double z2r = zr * zr - zi * zi;
        double z2i = 2.0 * zr * zi;

        // z^3 = z^2 * z
        double z3r = z2r * zr - z2i * zi;
        double z3i = z2r * zi + z2i * zr;

        // f = z^3 - 1
        double fr = z3r - 1.0;
        double fi = z3i;

        // f' = 3*z^2
        double fder_r = 3.0 * z2r;
        double fder_i = 3.0 * z2i;

        // f/f'
        double denom = fder_r * fder_r + fder_i * fder_i;
        double qr = (fr * fder_r + fi * fder_i) / denom;
        double qi = (fi * fder_r - fr * fder_i) / denom;

        zr = zr - qr;
        zi = zi - qi;
        iter++;

        if (zr * zr + zi * zi > 4.0)
            return 0xFF000000;

        for (int root = 0; root < 3; root++)
        {
            double dr = zr - raiz_r[root];
            double di = zi - raiz_i[root];
            if (dr * dr + di * di < 1e-4 * 1e-4)
            {
                int index = (root * 5 + iter) % PALETTE_SIZE;
                return color_ramp[index];
            }
        }
        iter++;
    }

    atomicAdd(device_iters, (unsigned long long)iter);
    return 0xFF000000; // negro
}

__global__ // funcion en la GPU
void newton_kernel(
    int num_iteraciones,
    double x_min, double y_min, double x_max, double y_max,
    uint32_t width,
    uint32_t heigt,
    uint32_t *pixel_buffer,
    unsigned long long *device_iters)
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
        auto color = acotado(x, y, num_iteraciones, device_iters);

        // index j*w + i
        pixel_buffer[j * width + i] = color;
    }
}

void copiar_paleta(unsigned int *pallete_host)
{
    // forma de copiar variables del host a la gpu  cudaMemcpyToSymbol-- copiar la paleta desde la cpu a la gpu
    cudaMemcpyToSymbol(color_ramp, pallete_host, sizeof(unsigned int) * PALETTE_SIZE);
}

void newton_gpu(
    int num_iteraciones,
    double x_min, double y_min, double x_max, double y_max,
    uint32_t width,
    uint32_t heigt,
    uint32_t *pixel_buffer,
    unsigned long long *device_iters)
{
    // limpiar contador en el device antes de cada frame
    cudaMemset(device_iters, 0, sizeof(unsigned long long));

    int threads_per_block = 1024;
    int blocks_per_grid = std::ceil((width * heigt) * 1.0 / threads_per_block);

    newton_kernel<<<blocks_per_grid, threads_per_block>>>(
        num_iteraciones,
        x_min, y_min, x_max, y_max,
        width, heigt,
        pixel_buffer, device_iters);
}