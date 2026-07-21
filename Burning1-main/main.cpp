#ifdef _WIN32
#include <windows.h>
#endif

#include <fmt/core.h>
#include <SFML/Graphics.hpp>
#include <omp.h>

#include <complex>
#include "fractal_Burning.h"

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
extern void burning_gpu(int num_iteraciones,
                         double x_min, double y_min, double x_max, double y_max,
                         uint32_t width, uint32_t height,
                         uint32_t *pixel_buffer, int *device_hist);
// =====================================================================================

// Pamarametro img
#define ANCHO 1600
#define ALTO 900

// Parameteros
int max_iteraciones = 100;
double x_min = -2.2;
double x_max = 1.2;

double y_min = -2.0;
double y_max = 1.2;
std::string texto;
int delta; // número de filas que cada proceso debe calcular, incluyendo el padding
int rows_start;
int rows_end;
int padding;

uint32_t *texture_buffer = nullptr;
uint32_t *pixel_buffer = nullptr;

//CUDA
uint32_t *device_pixel_buffer = nullptr;
int *device_hist = nullptr;
// =====================================================================================

std::vector<int> local_hist(16, 0);
std::vector<int> global_hist;

// uint16_t* pixel_buffer = nullptr;

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
    CHECK(cudaMalloc(&device_hist, 16 * sizeof(int)));
    copiar_paleta(color_ramp.data());
    //-------------------------

    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();

    sf::RenderWindow window(sf::VideoMode({ANCHO, ALTO}), "Burning");

#ifdef _WIN32
    HWND hwnd = window.getNativeHandle();
    ShowWindow(hwnd, SW_MAXIMIZE); // Maximizar Ventana
#endif

    sf::Texture texture(sf::Vector2u{ANCHO, ALTO});
    sf::Sprite sprite(texture);

    sf::Font font("arial.ttf");
    sf::Text text(font, "Burning Set", 24);
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
        std::fill(local_hist.begin(), local_hist.end(), 0);
        std::vector<int> hist(16, 0);
        if (r_type == runtime_type::SERIAL_1)
        {

            burning_serial(x_min, y_min, x_max, y_max, ANCHO, ALTO, pixel_buffer, hist);
            mode = "SERIAL 1";
        }
        else if (r_type == runtime_type::SIMD)
        {
            burning_simd(x_min, y_min, x_max, y_max, ANCHO, ALTO, pixel_buffer, hist);
            mode = "SIMD";
        }
        else if (r_type == runtime_type::OPENMP)
        {
            burning_omp(x_min, y_min, x_max, y_max, ANCHO, ALTO, pixel_buffer, hist);
            mode = "OPEN MP";
        }
        //CUDA
        else if (r_type == runtime_type::GPU_CUDA)
        {
            burning_gpu(max_iteraciones, x_min, y_min, x_max, y_max, ANCHO, ALTO,
            device_pixel_buffer, device_hist);
             CHECK(cudaGetLastError());
             CHECK(cudaMemcpy(pixel_buffer, device_pixel_buffer, ANCHO * ALTO * sizeof(uint32_t), cudaMemcpyDeviceToHost));
             mode = "GPU CUDA";
         }
        //----------------------------------------
        
        texto.clear();

        for (int i = 0; i < 16; i++)
        {
            texto += fmt::format("{}:{} ", i, hist[i]);
            if (i == 7)
                texto += "\n";
        }
        texture.update((const uint8_t *)pixel_buffer);

        frames++;

        if (clock.getElapsedTime().asSeconds() >= 1.0f)
        {
            fps = frames;
            frames = 0;
            clock.restart();
        }
        auto msg = fmt::format("Burning: iteraciones; {}.fps{}, Mode: {}, \n hist:{}", max_iteraciones, fps, mode, texto);
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
    cudaFree(device_hist);
    //----------------------------------------------------------------
    return 0;
}