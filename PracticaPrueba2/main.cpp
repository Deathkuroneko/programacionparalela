#include <SFML/Graphics.hpp>
#include <cstring>
#include <iostream>
#include <cstdint>
#include <immintrin.h>

#include "escala.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// =========================
// MAIN
// =========================
int main()
{
    int width, height, channels;

    // =========================
    // CARGA REAL DE IMAGEN
    // =========================
    uint8_t *rgba =
        stbi_load(
            "perro.jpg",
            &width,
            &height,
            &channels,
            STBI_rgb_alpha);

    if (!rgba)
    {
        std::cout << "NO SE PUDO CARGAR LA IMAGEN\n";
        return -1;
    }

    channels = 4;

    int total = width * height;

    uint32_t *pixel_buffer =
        new uint32_t[total];

    uint32_t *original_buffer =
        new uint32_t[total];

    // =========================
    // CONVERTIR RGBA -> uint32_t
    // Formato:
    // A B G R
    // =========================
    for (int i = 0; i < total; i++)
    {
        uint8_t r = rgba[i * 4 + 0];
        uint8_t g = rgba[i * 4 + 1];
        uint8_t b = rgba[i * 4 + 2];
        uint8_t a = rgba[i * 4 + 3];

        uint32_t px =
            (a << 24) |
            (b << 16) |
            (g << 8) |
            r;

        original_buffer[i] = px;
        pixel_buffer[i] = px;
    }

    stbi_image_free(rgba); // Liberar buffer original de stb_image

    // =========================
    // SFML
    // =========================
    sf::RenderWindow window(
        sf::VideoMode(
            {
                (unsigned int)width,
                (unsigned int)height
            }),
        "SIMD Gray Filter");

    sf::Texture texture(
        {
            (unsigned int)width,
            (unsigned int)height
        });

    sf::Sprite sprite(texture);

    // =========================
    // MODOS
    // =========================
    enum class Mode
    {
        ORIGINAL = 1,
        GRAYSCALEAVX = 2,
        GRAYSCALEOPENMP = 3
    };

    Mode mode = Mode::ORIGINAL;

    // =========================
    // LOOP
    // =========================
    while (window.isOpen())
    {
        while (auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (event->is<sf::Event::KeyReleased>())
            {
                auto evt =
                    event->getIf<sf::Event::KeyReleased>();

                // =========================
                // CAMBIO DE MODOS
                // =========================
                if (evt->scancode ==
                    sf::Keyboard::Scan::Num1)
                {
                    mode = Mode::ORIGINAL;
                }

                if (evt->scancode ==
                    sf::Keyboard::Scan::Num2)
                {
                    mode = Mode::GRAYSCALEAVX;
                }

                if (evt->scancode ==
                    sf::Keyboard::Scan::Num3)
                {
                    mode = Mode::GRAYSCALEOPENMP;
                }

                // =========================
                // GUARDAR IMAGEN
                // =========================
                if (evt->scancode ==
                    sf::Keyboard::Scan::S)
                {
                    // --------------------------------
                    // CREAR BUFFER GRIS REAL (1 canal)
                    // --------------------------------
                    uint8_t* gray_pixels =
                        new uint8_t[total];

                    for (int i = 0; i < total; i++)
                    {
                        uint32_t px =
                            pixel_buffer[i];

                        // Tomamos R
                        // porque R=G=B
                        gray_pixels[i] =
                            px & 0xFF;
                    }

                    // --------------------------------
                    // GUARDAR SEGÚN MODO
                    // --------------------------------
                    if (mode ==
                        Mode::GRAYSCALEAVX)
                    {
                        stbi_write_png(
                            "salida_simd.png",
                            width,
                            height,
                            STBI_grey,
                            gray_pixels,
                            width);

                        std::cout
                            << "Guardado: salida_simd.png\n";
                    }

                    else if (mode ==
                             Mode::GRAYSCALEOPENMP)
                    {
                        stbi_write_png(
                            "salida_openmp.png",
                            width,
                            height,
                            STBI_grey,
                            gray_pixels,
                            width);

                        std::cout
                            << "Guardado: salida_openmp.png\n";
                    }

                    else
                    {
                        stbi_write_png(
                            "salida_original.png",
                            width,
                            height,
                            STBI_grey,
                            gray_pixels,
                            width);

                        std::cout
                            << "Guardado: salida_original.png\n";
                    }

                    delete[] gray_pixels;
                }
            }
        }

        // =========================
        // PIPELINE
        // =========================
        std::memcpy(
            pixel_buffer,
            original_buffer,
            total * sizeof(uint32_t));

        if (mode == Mode::GRAYSCALEAVX)
        {
            Gray_SIMD(
                pixel_buffer,
                width,
                height);
        }

        if (mode == Mode::GRAYSCALEOPENMP)
        {
            Gray_OPENMP(
                pixel_buffer,
                width,
                height);
        }

        texture.update(
            (const uint8_t*)pixel_buffer);

        window.clear();

        window.draw(sprite);

        window.display();
    }

    delete[] pixel_buffer;
    delete[] original_buffer;

    return 0;
}