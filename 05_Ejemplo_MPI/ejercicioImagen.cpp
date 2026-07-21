#include <mpi.h>
#include <vector>
#include <math.h>
#include <fmt/core.h>

using namespace std;

// imagen

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void escalaGris(uint32_t *pixelOriginal, int total, uint32_t *pixelGris)
{
    for (int i = 0; i < total; i++)
    {
        uint32_t pixel = pixelOriginal[i];
        uint32_t r = (pixel) & 0xFF;
        uint32_t g = (pixel >> 8) & 0xFF;
        uint32_t b = (pixel >> 16) & 0xFF;
        uint32_t a = (pixel >> 24) & 0xFF;
        int gray = (0.21 * r) + (0.72 * g) + (0.07 * b);

        uint32_t nPixel = (a << 24) | (gray << 16) | (gray << 8) | (gray);

        pixelGris[i] = nPixel;
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
        int width, height, channels;
        uint8_t *rgba_pixels =
            stbi_load("img.jpg", &width, &height, &channels, STBI_rgb_alpha);
        channels = 4;
        // recuperar rgb
        int total = width * height;
        uint32_t *pixelBuffer = new uint32_t[total];
        for (int i = 0; i < total; i++)
        {
            uint32_t r = rgba_pixels[i * 4 + 0];
            uint32_t g = rgba_pixels[i * 4 + 1];
            uint32_t b = rgba_pixels[i * 4 + 2];
            uint32_t a = rgba_pixels[i * 4 + 3];
            uint32_t pixel = (r) | (g << 8) | (b << 16) | (a << 24);

            pixelBuffer[i] = pixel;
        }
        int pixelProcesar = ceil((total * 1.0) / nprocs);
        for (int i = 1; i < nprocs; i++)
        {
            int nData = pixelProcesar;
            if (i == nprocs - 1)
            {
                nData = (total - ((nprocs - 1) * pixelProcesar));
            }
            MPI_Send(
                &nData,
                1,
                MPI_INT,
                i,
                0,
                MPI_COMM_WORLD);
            // enviar datos
            MPI_Send(
                pixelBuffer + i * nData,
                nData,
                MPI_UINT32_T,
                i,
                0,
                MPI_COMM_WORLD);
        }
         // procesar los datos
            uint32_t *bufferTemp = new uint32_t[pixelProcesar];
            for (int i = 0; i < pixelProcesar; i++)
            {
                bufferTemp[i] = pixelBuffer[i];
            }
            uint32_t *escalaTemp = new uint32_t[pixelProcesar];
            escalaGris(bufferTemp, pixelProcesar, escalaTemp);
            // recibir las partes
            uint32_t *escalaGrises = new uint32_t[total];
            for (int i = 0; i < pixelProcesar; i++)
            {
                escalaGrises[i] = escalaTemp[i];
            }
            for (int i = 1; i < nprocs; i++)
            {
                int nData = pixelProcesar;
                if (i == nprocs - 1)
                {
                    nData = (total - ((nprocs - 1) * pixelProcesar));
                }
                MPI_Recv(
                    escalaGrises+i*nData,
                    nData,
                    MPI_UINT32_T,
                    i,
                    0,
                    MPI_COMM_WORLD,
                    MPI_STATUS_IGNORE
                );
            }
            //crear imagen en gris
            stbi_write_png("Imagen_gris.png", width, height, 4, escalaGrises, width * 4);
    }
    else
    {
        int nData = 0;
        MPI_Recv(
            &nData,
            1,
            MPI_INT,
            0,
            0,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE);
        uint32_t *bufferProcesar = new uint32_t[nData];
        MPI_Recv(
            bufferProcesar,
            nData,
            MPI_UINT32_T,
            0,
            0,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE);
        uint32_t *bufferGris = new uint32_t[nData];

        escalaGris(bufferProcesar, nData, bufferGris);

        MPI_Send(
            bufferGris,
            nData,
            MPI_UINT32_T,
            0,
            0,
            MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
