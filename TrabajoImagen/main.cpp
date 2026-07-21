#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <SFML/Graphics.hpp>
#include <immintrin.h>
#include <omp.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

#ifdef _WIN32
    #include <windows.h>
#endif

enum class ViewMode {
    ORIGINAL = 0,
    SIMD,
    OPENMP
};

// Operaciones vectoriales (vectores intrínsecos)
void aplicarFiltroSIMD(const uint8_t* src, uint8_t* dst,
    int width, int height)
{
    int total_pixels = width * height;

    // coeficientes de luminancia
    __m256 rCoeficiente = _mm256_set1_ps(0.21f);
    __m256 gCoeficiente = _mm256_set1_ps(0.72f);
    __m256 bCoeficiente = _mm256_set1_ps(0.07f);

    int i = 0;

    // Procesar 8 píxeles
    for (; i + 7 < total_pixels; i += 8)
    {
        alignas(32) float r[8];
        alignas(32) float g[8];
        alignas(32) float b[8];

        // Extraer canales
        for (int p = 0; p < 8; ++p)
        {
            int idx = (i + p) * 4;

            r[p] = (float)src[idx + 0];
            g[p] = (float)src[idx + 1];
            b[p] = (float)src[idx + 2];
        }

        // Cargar canales en vectores
        __m256 vr = _mm256_load_ps(r);
        __m256 vg = _mm256_load_ps(g);
        __m256 vb = _mm256_load_ps(b);

        // gray_scale = 0.21R + 0.72G + 0.07B
        __m256 gray = _mm256_add_ps(
            _mm256_add_ps(
                _mm256_mul_ps(vr, rCoeficiente),
                _mm256_mul_ps(vg, gCoeficiente)
            ),
            _mm256_mul_ps(vb, bCoeficiente)
        );

        // float a int
        __m256i grayInt = _mm256_cvtps_epi32(gray);

        // guardar resultado temporal
        alignas(32) int grayVals[8];
        _mm256_store_si256((__m256i*)grayVals, grayInt);

        // RGBA
        for (int p = 0; p < 8; ++p)
        {
            int idx = (i + p) * 4;
            uint8_t g = (uint8_t)grayVals[p];

            dst[idx + 0] = g;
            dst[idx + 1] = g;
            dst[idx + 2] = g;
            dst[idx + 3] = 255;
        }
    }

    // píxeles restantes
    for (; i < total_pixels; ++i)
    {
        int idx = i * 4;

        float gray =
            0.21f * src[idx + 0] +
            0.72f * src[idx + 1] +
            0.07f * src[idx + 2];

        uint8_t g = (uint8_t)gray;

        dst[idx + 0] = g;
        dst[idx + 1] = g;
        dst[idx + 2] = g;
        dst[idx + 3] = 255;
    }
}

// OpenMP, utilizar una distribución manual de bloques (no utilizar parallel for)
void aplicarFiltroOpenMP(const uint8_t* src, uint8_t* dst, int width, int height) {
    int total_pixels = width * height;

    #pragma omp parallel
    {
        int thread_count = omp_get_num_threads();
        int thread_id = omp_get_thread_num();
        
        // Distribución manual por bloques
        int delta = std::ceil(total_pixels * 1.0 / thread_count);
        int start = thread_id * delta;
        int end = std::min((thread_id + 1) * delta, total_pixels);

        for (int i = start; i < end; ++i) {
            float r = src[i * 4 + 0];
            float g = src[i * 4 + 1];
            float b = src[i * 4 + 2];
            
            uint8_t gray = static_cast<uint8_t>(0.21f * r + 0.72f * g + 0.07f * b);
            
            dst[i * 4 + 0] = gray; // R
            dst[i * 4 + 1] = gray; // G
            dst[i * 4 + 2] = gray; // B
            dst[i * 4 + 3] = 255;  // A
        }
    }
}

// Guardar en la pc / 1 Canal de escala de grises
void guardarImagenGris(const std::string& filename, const uint8_t* rgba_buffer, int width, int height) {
    std::vector<uint8_t> gray_pixels(width * height);
    
    // Convertimos el buffer RGBA de visualización a un buffer puro de 1 canal (Grays_cale)
    for (int i = 0; i < width * height; ++i) {
        gray_pixels[i] = rgba_buffer[i * 4]; 
    }
    
    stbi_write_png(filename.c_str(), width, height, STBI_grey, gray_pixels.data(), width);
    std::cout << "Imagen guardada exitosamente como: " << filename << std::endl;
}

int main() {
    // cargar img.jpg con STB
    int width, height, channels;
    uint8_t* rgba_pixeles = stbi_load("img.jpg", &width, &height, &channels, STBI_rgb_alpha);
    
    if (!rgba_pixeles) {
        std::cerr << "Error: No se pudo cargar la imagen 'img.jpg'" << std::endl;
        return -1;
    }

    // Buffers para almacenar los resultados
    std::vector<uint8_t> buffer_simd(width * height * 4);
    std::vector<uint8_t> buffer_openmp(width * height * 4);
    std::vector<uint8_t> buffer_render(width * height * 4);

    // los filtros al inicio
    aplicarFiltroSIMD(rgba_pixeles, buffer_simd.data(), width, height);
    aplicarFiltroOpenMP(rgba_pixeles, buffer_openmp.data(), width, height);

    // imagen original
    std::copy(rgba_pixeles, rgba_pixeles + (width * height * 4), buffer_render.begin());
    ViewMode current_mode = ViewMode::ORIGINAL;

    // Ventana SFML
    sf::RenderWindow window(sf::VideoMode({static_cast<unsigned int>(width), static_cast<unsigned int>(height)}), "Filtro Escala de Grises - SIMD & OpenMP");
    
#ifdef _WIN32
    HWND hwnd = window.getNativeHandle();
    ShowWindow(hwnd, SW_MAXIMIZE); 
#endif

    sf::Texture texture({static_cast<unsigned int>(width), static_cast<unsigned int>(height)});
    sf::Sprite sprite(texture);

    // Fuentes y Textos
    sf::Font font("arial.ttf");
    sf::Text textInfo(font, "Modo: Original", 22);
    textInfo.setFillColor(sf::Color::Green);
    textInfo.setPosition({15, 15});
    textInfo.setStyle(sf::Text::Bold);

    sf::Text textControls(font, "[1] Original  [2] SIMD (AVX2)  [3] OpenMP  [S] Guardar Imagen", 18);
    textControls.setFillColor(sf::Color::White);
    textControls.setPosition({15, window.getView().getSize().y - 40});
    textControls.setStyle(sf::Text::Bold);

    // Loop Principal
    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            else if (event->is<sf::Event::KeyReleased>()) {
                auto evt = event->getIf<sf::Event::KeyReleased>();
                
                switch (evt->scancode) {
                    case sf::Keyboard::Scan::Num1:
                        current_mode = ViewMode::ORIGINAL;
                        std::copy(rgba_pixeles, rgba_pixeles + (width * height * 4), buffer_render.begin());
                        textInfo.setString("Modo: Original");
                        break;
                        
                    case sf::Keyboard::Scan::Num2:
                        current_mode = ViewMode::SIMD;
                        std::copy(buffer_simd.begin(), buffer_simd.end(), buffer_render.begin());
                        textInfo.setString("Modo: SIMD grises (AVX2)");
                        break;
                        
                    case sf::Keyboard::Scan::Num3:
                        current_mode = ViewMode::OPENMP;
                        std::copy(buffer_openmp.begin(), buffer_openmp.end(), buffer_render.begin());
                        textInfo.setString("Modo: OpenMP grises");
                        break;
                        
                    case sf::Keyboard::Scan::S:
                        if (current_mode == ViewMode::SIMD) {
                            guardarImagenGris("salida_simd.png", buffer_simd.data(), width, height);
                        } else if (current_mode == ViewMode::OPENMP) {
                            guardarImagenGris("salida_openmp.png", buffer_openmp.data(), width, height);
                        } else {
                            std::cout << "Cambia al modo SIMD o OpenMP antes de guardar." << std::endl;
                        }
                        break;
                    default:
                        break;
                }
            }
        }

        // textura con el buffer
        texture.update(buffer_render.data());

        // Renderizado
        window.clear();
        window.draw(sprite);
        window.draw(textInfo);
        window.draw(textControls);
        window.display();
    }

    // Limpieza de memoria
    stbi_image_free(rgba_pixeles);
    return 0;
}