#include <iostream>
#include <fmt/core.h>
#include <vector>
#include <math.h>
#include <mpi.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using namespace std;

void escalaGrisesFunction(uint32_t *pixelbuffer, int total, uint32_t *grissbuffer)
{
    for (int i = 0; i < total; i++)
    {
        uint32_t pixel = pixelbuffer[i];
        uint32_t a = (pixel >> 24) & 0xff;
        uint32_t r = (pixel >> 16) & 0xff;
        uint32_t g = (pixel >> 8) & 0xff;
        uint32_t b = (pixel >> 0) & 0xff;

        int gris = (0.21 * r) + (0.72 * g) + (0.07 * b);

        uint32_t nPixel = (a << 24) | (gris << 16) | (gris << 8) | (gris << 0);
        grissbuffer[i] = nPixel;
    }
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int rank, nprocs;
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // cargar imagen
    if (rank == 0)
    {
        // cargar imagen rank 0
        int width, height, channels;
        uint8_t *rgba_pixels =
            stbi_load("img.jpg", &width, &height, &channels, STBI_rgb_alpha);
        channels = 4;

        int total = width * height;

        uint32_t *pixelBuffer = new uint32_t[total];
        for (int i = 0; i < total; i++)
        {
            uint32_t r = rgba_pixels[i * 4 + 0];
            uint32_t g = rgba_pixels[i * 4 + 1];
            uint32_t b = rgba_pixels[i * 4 + 2];
            uint32_t a = rgba_pixels[i * 4 + 3];

            uint32_t pixel = (a << 24) | (r << 16) | (g << 8) | (b);
            pixelBuffer[i] = pixel;
        }
        // dividir trabajos
        int datos_per_rank = ceil((total * 1.0) / nprocs);
        for (int i = 1; i < nprocs; i++)
        {
            // separacion para el ultimo rank
            int nDatos = datos_per_rank;
            if (i == (nprocs - 1))
            {
                nDatos = total - ((nprocs - 1) * datos_per_rank);
            }
            // ultimo rank
            // envio del numero de datos
            MPI_Send(
                &nDatos,
                1,
                MPI_INT,
                i,
                0,
                MPI_COMM_WORLD);
            // envio de los datos
            MPI_Send(
                pixelBuffer + i * nDatos,
                nDatos,
                MPI_UINT32_T,
                i,
                0,
                MPI_COMM_WORLD);
        }
        // REALIZAR PARTE DEL RANK 0
        uint32_t *grisBuffer = new uint32_t[datos_per_rank];
        escalaGrisesFunction(pixelBuffer, datos_per_rank, grisBuffer);

        uint32_t *grayScaleBuffer = new uint32_t[total];

        for (int i = 0; i < datos_per_rank; i++)
        {
            grayScaleBuffer[i] = grisBuffer[i];
        }
        // recibir porcion de otros ranks

        for (int i = 1; i < nprocs; i++)
        {
            // separacion para el ultimo rank
            int nDatos = datos_per_rank;
            if (i == (nprocs - 1))
            {
                nDatos = total - ((nprocs - 1) * datos_per_rank);
            }

            MPI_Recv(
                grayScaleBuffer + i * nDatos,
                nDatos,
                MPI_UINT32_T,
                i,
                0,
                MPI_COMM_WORLD,
                MPI_STATUS_IGNORE);
        }
        stbi_write_png("img_gris.png", width, height, 4,
                       grayScaleBuffer, width*4);
    }
    // parte de los otros ranks
    else
    {
        int nDatos;
        MPI_Recv(
            &nDatos,
            1,
            MPI_INT,
            0,
            0,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE);
        uint32_t *bufferParcial = new uint32_t[nDatos];
        // recibir pixel
        MPI_Recv(
            bufferParcial,
            nDatos,
            MPI_UINT32_T,
            0,
            0,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE);

        uint32_t *bufferGris = new uint32_t[nDatos];

        escalaGrisesFunction(bufferParcial, nDatos, bufferGris);

        // enviar parte procesada

        MPI_Send(
            bufferGris,
            nDatos,
            MPI_UINT32_T,
            0,
            0,
            MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
