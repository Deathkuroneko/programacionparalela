#include <fmt/core.h>
#include <vector>
#include <cmath>
#include <mpi.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using namespace std;

void escalaGrisesFuncion(uint32_t *pixelBuffer, int numeroDatos, uint32_t *escalaGrises)
{
    for (int i = 0; i < numeroDatos; i++)
    {
        uint32_t pixel = pixelBuffer[i];

        uint32_t a = (pixel >> 24) & 0xff;
        uint32_t r = (pixel >> 16) & 0xff;
        uint32_t g = (pixel >> 8) & 0xff;
        uint32_t b = pixel & 0xff;

        int gray = static_cast<int>(r * 0.21 + g * 0.72 + b * 0.07);

        uint32_t nuevoPixel =
            (a << 24) |
            (gray << 16) |
            (gray << 8) |
            gray;

        escalaGrises[i] = nuevoPixel;
    }
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int width = 0;
    int height = 0;
    int channels = 0;

    int total = 0;
    int datosProcesar = 0;
    int padded_rows = 0;

    uint32_t *pixelbuffer = nullptr;
    uint32_t *pixelBufferPadded = nullptr;
    uint32_t *grayBuffer = nullptr;

    //-------------------------------------------------------
    // El proceso maestro carga la imagen
    //-------------------------------------------------------

    if (rank == 0)
    {
        uint8_t *rgba_pixels =
            stbi_load("img.jpg",
                      &width,
                      &height,
                      &channels,
                      STBI_rgb_alpha);

        if (rgba_pixels == nullptr)
        {
            fmt::print("Error: no se pudo abrir img.jpg\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        channels = 4;

        total = width * height;

        pixelbuffer = new uint32_t[total];

        for (int i = 0; i < total; i++)
        {
            uint32_t r = rgba_pixels[i * 4 + 0];
            uint32_t g = rgba_pixels[i * 4 + 1];
            uint32_t b = rgba_pixels[i * 4 + 2];
            uint32_t a = rgba_pixels[i * 4 + 3];

            pixelbuffer[i] =
                (a << 24) |
                (r << 16) |
                (g << 8) |
                b;
        }

        stbi_image_free(rgba_pixels);

        datosProcesar = static_cast<int>(ceil(total * 1.0 / nprocs));
        padded_rows = datosProcesar * nprocs;

        pixelBufferPadded = new uint32_t[padded_rows]();

        for (int i = 0; i < total; i++)
        {
            pixelBufferPadded[i] = pixelbuffer[i];
        }
    }

    //-------------------------------------------------------
    // Compartir información con todos los procesos
    //-------------------------------------------------------

    MPI_Bcast(&width, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&height, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&channels, 1, MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Bcast(&datosProcesar, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&padded_rows, 1, MPI_INT, 0, MPI_COMM_WORLD);

    //-------------------------------------------------------
    // Scatter
    //-------------------------------------------------------

    uint32_t *dataTMP = new uint32_t[datosProcesar];

    MPI_Scatter(
        pixelBufferPadded,
        datosProcesar,
        MPI_UINT32_T,
        dataTMP,
        datosProcesar,
        MPI_UINT32_T,
        0,
        MPI_COMM_WORLD);

    //-------------------------------------------------------
    // Escala de grises
    //-------------------------------------------------------

    uint32_t *grayBufferTmp = new uint32_t[datosProcesar];

    escalaGrisesFuncion(
        dataTMP,
        datosProcesar,
        grayBufferTmp);

    //-------------------------------------------------------
    // Gather
    //-------------------------------------------------------

    if (rank == 0)
    {
        grayBuffer = new uint32_t[padded_rows];
    }

    MPI_Gather(
        grayBufferTmp,
        datosProcesar,
        MPI_UINT32_T,
        grayBuffer,
        datosProcesar,
        MPI_UINT32_T,
        0,
        MPI_COMM_WORLD);

    //-------------------------------------------------------
    // Guardar imagen
    //-------------------------------------------------------

    if (rank == 0)
    {
        int ok = stbi_write_png(
            "img-gris.png",
            width,
            height,
            4,
            grayBuffer,
            width * 4);

        if (ok)
            fmt::print("Imagen guardada correctamente.\n");
        else
            fmt::print("Error al guardar la imagen.\n");
    }

    //-------------------------------------------------------
    // Liberar memoria
    //-------------------------------------------------------

    delete[] dataTMP;
    delete[] grayBufferTmp;

    if (rank == 0)
    {
        delete[] pixelbuffer;
        delete[] pixelBufferPadded;
        delete[] grayBuffer;
    }

    MPI_Finalize();

    return 0;
}