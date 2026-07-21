#ifdef _WIN32
#include <windows.h>
#endif

#include <fmt/core.h>
#include <SFML/Graphics.hpp>
#include <omp.h>

#include <complex>
#include "fractal_Newton.h"

//CUDA
#include <cuda_runtime.h>
#include "palette.h"

#define CHECK(expr)                                                                                                               \
     {                                                                                                                             \
         auto internal_error = (expr);                                                                                             \
         if (internal_error != cudaSuccess)                                                                                        \
         {                                                                                                                         \
             fmt::println("{}: {} in {} at line {}", (int)internal_error, cudaGetErrorString(internal_error), __FILE__, __LINE__); \
             exit(EXIT_FAILURE);                                                                                                   \
         }                                                                                                                         \
     }
extern void copiar_paleta(unsigned int *pallete_host);
extern void newton_gpu(int num_iteraciones,
                        double x_min, double y_min, double x_max, double y_max,
                        uint32_t width, uint32_t height,
                        uint32_t *pixel_buffer, unsigned long long *device_iters);
// =====================================================================================

// Pamarametro img
#define ANCHO 1600
#define ALTO 900

// Parameteros
int max_iteraciones = 100;
double x_min = -1.5;
double x_max = 1.5;

double y_min = -1.0;
double y_max = 1.0;
std::string texto;

uint32_t *texture_buffer = nullptr;
uint32_t *pixel_buffer = nullptr;

//CUDA
uint32_t *device_pixel_buffer = nullptr;
unsigned long long *device_iters = nullptr;
// =====================================================================================

long long total_iters = 0;

enum class runtime_type
{
    SERIAL_1 = 0,
    SIMD,
    OPENMP,
    OPENMPFOR,
    OPENMPFORSIMD,
    GPU_CUDA //modo CUDA
};

int main()
{

    texture_buffer = new uint32_t[ANCHO * ALTO];
    std::memset(texture_buffer, 0, ANCHO * ALTO * sizeof(uint32_t));

    runtime_type r_type = runtime_type::SERIAL_1;
    pixel_buffer = new uint32_t[ANCHO * ALTO];

    // CUDA
    //-------------------------
    int deviceId = 0;
    cudaSetDevice(deviceId);
    cudaDeviceProp deviceProp;
    cudaGetDeviceProperties(&deviceProp, deviceId);
    size_t buffer_size = ANCHO * ALTO * sizeof(uint32_t);
    CHECK(cudaMalloc(&device_pixel_buffer, buffer_size));
    CHECK(cudaMalloc(&device_iters, sizeof(unsigned long long)));
    copiar_paleta(color_ramp.data());
    //-------------------------

    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();

    sf::RenderWindow window(sf::VideoMode({ANCHO, ALTO}), "Newton");

#ifdef _WIN32
    HWND hwnd = window.getNativeHandle();
    ShowWindow(hwnd, SW_MAXIMIZE); // Maximizar Ventana
#endif

    sf::Texture texture(sf::Vector2u{ANCHO, ALTO});
    sf::Sprite sprite(texture);

    sf::Font font("arial.ttf");
    sf::Text text(font, "Newton Fractal", 24);
    text.setFillColor(sf::Color::White);
    text.setPosition({10, 10});
    text.setStyle(sf::Text::Bold);

    std::string options = "Options: [1]Serial [2]SIMD  [3]OPENMP [4]CUDA |  Up/Sown change iterations";
    sf::Text textoptions(font, options, 20);

    textoptions.setStyle(sf::Text::Bold);
    textoptions.setPosition({10, window.getView().getSize().y - 40});
    // FPS
    int frames = 0;
    int fps = 0;
    sf::Clock clock;

    // Start the game loop
    while (window.isOpen())
    {
        // Process events
        while (const std::optional event = window.pollEvent())
        {
            // Close window: exit
            if (event->is<sf::Event::Closed>())
                window.close();
            else if (event->is<sf::Event::KeyReleased>())
            {
                auto evt = event->getIf<sf::Event::KeyReleased>();

                switch (evt->scancode)
                {
                case sf::Keyboard::Scan::Up:
                    max_iteraciones += 10;
                    break;
                case sf::Keyboard::Scan::Down:
                    max_iteraciones -= 10;
                    if (max_iteraciones < 10)
                        max_iteraciones = 10;
                    break;
                case sf::Keyboard::Scan::Num1:
                    r_type = runtime_type::SERIAL_1;
                    break;
                case sf::Keyboard::Scan::Num2:
                    r_type = runtime_type::SIMD;
                    break;
                case sf::Keyboard::Scan::Num3:
                    r_type = runtime_type::OPENMP;
                    break;
                case sf::Keyboard::Scan::Num4:
                     r_type = runtime_type::GPU_CUDA;//<-- CUDA
                     break;
                }
            }
        }
        std::string mode = "";
        // Dibuja Fractal dependera de la velocidad
        total_iters = 0;
        if (r_type == runtime_type::SERIAL_1)
        {

            newton_serial(x_min, y_min, x_max, y_max, ANCHO, ALTO, pixel_buffer, total_iters);
            mode = "SERIAL 1";
        }
        else if (r_type == runtime_type::SIMD)
        {
            newton_simd(x_min, y_min, x_max, y_max, ANCHO, ALTO, pixel_buffer, total_iters);
            mode = "SIMD";
        }
        else if (r_type == runtime_type::OPENMP)
        {
            newton_omp(x_min, y_min, x_max, y_max, ANCHO, ALTO, pixel_buffer, total_iters);
            mode = "OPEN MP";
        }
        //CUDA
        else if (r_type == runtime_type::GPU_CUDA)
        {
            newton_gpu(max_iteraciones, x_min, y_min, x_max, y_max, ANCHO, ALTO,
            device_pixel_buffer, device_iters);
             CHECK(cudaGetLastError());
             CHECK(cudaMemcpy(pixel_buffer, device_pixel_buffer, ANCHO * ALTO * sizeof(uint32_t), cudaMemcpyDeviceToHost));

             unsigned long long iters_host = 0;
             CHECK(cudaMemcpy(&iters_host, device_iters, sizeof(unsigned long long), cudaMemcpyDeviceToHost));
             total_iters = (long long)iters_host;
             mode = "GPU CUDA";
         }
        //----------------------------------------

        texture.update((const uint8_t *)pixel_buffer);

        frames++;

        if (clock.getElapsedTime().asSeconds() >= 1.0f)
        {
            fps = frames;
            frames = 0;
            clock.restart();
        }
        auto msg = fmt::format("Newton: iteraciones; {}.fps{}, Mode: {}, \n total_iters:{}", max_iteraciones, fps, mode, total_iters);
        text.setString(msg);

        // Clear screen
        window.clear();
        {
            window.draw(sprite);
            window.draw(text);
            window.draw(textoptions);
        }

        // Update the window
        window.display();
    }

    delete[] pixel_buffer;

    //CUDA
    cudaFree(device_pixel_buffer);
    cudaFree(device_iters);
    //----------------------------------------------------------------
    return 0;
}