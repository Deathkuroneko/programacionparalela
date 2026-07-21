#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
#include <cmath>
#include <cstring>
#include <fmt/core.h>
#include <SFML/Graphics.hpp>
#include "fractal_mpi_newton.h"
#include "mpi.h"
#include "arial_ttf.h"
#include "draw_text.h"

std::string machine_name()
{
    std::string mname = "";
#ifdef _WIN32
    char hostname[256];
    DWORD size = sizeof(hostname);
    GetComputerNameA(hostname, &size);
    mname = hostname;
#endif
    return mname;
}

int max_iteraciones = 50;
double x_min = -1.5;
double x_max = 1.5;
double y_min = -1.0;
double y_max = 1.0;

uint32_t *pixel_buffer = nullptr;
uint32_t *texture_buffer = nullptr;
int running = 1;
int delta;      
int rows_start;
int rows_end;
int nprocs, rank;
int padding;

#define ANCHO 1600
#define ALTO 900

struct BcastParams
{
    int max_iter;
    int running;
    double x_min;
    double x_max;
    double y_min;
    double y_max;
};



void setup_ui()
{
    texture_buffer = new uint32_t[ANCHO * ALTO];
    std::memset(texture_buffer, 0, ANCHO * ALTO * sizeof(uint32_t));

    sf::RenderWindow window(sf::VideoMode({ANCHO, ALTO}), "Fractal de Newton - MPI");

#ifdef _WIN32
    HWND hwnd = window.getNativeHandle();
    ShowWindow(hwnd, SW_MAXIMIZE);
#endif

    sf::Texture texture({ANCHO, ALTO});
    texture.update((const uint8_t *)texture_buffer);
    sf::Sprite sprite(texture);

    sf::Font font(arial_ttf::data, arial_ttf::data_len);


    sf::Text text(font, "Fractal de Newton ", 22);
    text.setFillColor(sf::Color::White);
    text.setPosition({10, 10});
    text.setStyle(sf::Text::Bold);

    std::string options = "Up/Down: cambiar max_iter   Esc: salir";
    sf::Text textoptions(font, options, 20);
    textoptions.setFillColor(sf::Color::White);
    textoptions.setStyle(sf::Text::Bold);
    textoptions.setPosition({10, window.getView().getSize().y - 40});

    int frames = 0;
    int fps = 0;
    sf::Clock clock;

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                running = 0;
                window.close();
            }
            else if (event->is<sf::Event::KeyReleased>())
            {
                auto evt = event->getIf<sf::Event::KeyReleased>();

                switch (evt->scancode)
                {
                case sf::Keyboard::Scan::Up:
                    max_iteraciones += 10;
                    if (max_iteraciones > 50)
                        max_iteraciones = 50;
                    break;
                case sf::Keyboard::Scan::Down:
                    max_iteraciones -= 10;
                    if (max_iteraciones < 10)
                        max_iteraciones = 10;
                    break;
                case sf::Keyboard::Scan::Escape:
                    running = 0;    
                    window.close();
                    break;
                default:
                    break;
                }
                std::memset(texture_buffer, 0, ANCHO * ALTO * sizeof(uint32_t));
            }
        }

        // Difundir parámetros a los demás ranks
        BcastParams params{max_iteraciones, running, x_min, x_max, y_min, y_max};
        MPI_Bcast(&params, sizeof(BcastParams), MPI_BYTE, 0, MPI_COMM_WORLD);

        if (running == 0)
        {
            break;
        }

        //Cómputo de la franja propia de rank 0
        double compute_ms_local = 0.0;
        long total_iters_local = 0;
        newton_mpi(x_min, y_min, x_max, y_max, ANCHO, ALTO, rows_start, rows_end,
                pixel_buffer, compute_ms_local, total_iters_local);
        std::memcpy(texture_buffer, pixel_buffer, ANCHO * delta * sizeof(uint32_t));

        //Recibir las porciones de imagen de los demás procesos
        for (int i = 1; i < nprocs; i++)
        {
            int new_delta = delta;
            if (i == nprocs - 1)
            {
                new_delta = delta - padding;
            }
            MPI_Status status;
            MPI_Recv(
                pixel_buffer,
                ANCHO * new_delta,
                MPI_UNSIGNED,
                i,
                0,
                MPI_COMM_WORLD,
                &status);
            std::memcpy(texture_buffer + i * delta * ANCHO, pixel_buffer, ANCHO * new_delta * sizeof(uint32_t));
        }

        //Colectivas de estadísticas
        double max_compute_ms = 0.0;
        long total_iters = 0;
        MPI_Reduce(&compute_ms_local, &max_compute_ms, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
        MPI_Reduce(&total_iters_local, &total_iters, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
        

        texture.update((const uint8_t *)texture_buffer);

        frames++;
        if (clock.getElapsedTime().asSeconds() >= 1.0f)
        {
            fps = frames;
            frames = 0;
            clock.restart();
        }

        auto msg = fmt::format(
            "RANK {} | max_iter: {} | max_compute: {:.2f} ms | total_iters: {} | fps: {} | Maquina: {}",
            rank, max_iteraciones, max_compute_ms, total_iters, fps, machine_name());
        text.setString(msg);

        window.clear();
        {
            window.draw(sprite);
            window.draw(text);
            window.draw(textoptions);
        }
        window.display();
    }
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    init_freetype();

    delta = std::ceil(1.0 * ALTO / nprocs);
    rows_start = rank * delta;
    rows_end = rows_start + delta;
    padding = delta * nprocs - ALTO;

    if (rows_end > ALTO)
    {
        rows_end = ALTO;
    }

    pixel_buffer = new uint32_t[ANCHO * delta];
    std::memset(pixel_buffer, 0, ANCHO * delta * sizeof(uint32_t));
    fmt::print("Rank {}: rows {} to {}\n", rank, rows_start, rows_end);

    if (rank == 0)
    {
        setup_ui();
    }
    else
    {
        while (true)
        {
            BcastParams params{};
            MPI_Bcast(&params, sizeof(BcastParams), MPI_BYTE, 0, MPI_COMM_WORLD);

            max_iteraciones = params.max_iter;
            running = params.running;
            x_min = params.x_min;
            x_max = params.x_max;
            y_min = params.y_min;
            y_max = params.y_max;

            if (running == 0)
            {
                fmt::print("Rank {}: received shutdown signal, exiting...\n", rank);
                break;
            }

            double compute_ms_local = 0.0;
            long total_iters_local = 0;
            newton_mpi(x_min, y_min, x_max, y_max, ANCHO, ALTO, rows_start, rows_end,
                       pixel_buffer, compute_ms_local, total_iters_local);

            int send_rows = rows_end - rows_start;
            MPI_Send(
                pixel_buffer,
                ANCHO * send_rows,
                MPI_UNSIGNED,
                0,
                0,
                MPI_COMM_WORLD);

            // participa todo el rank
            MPI_Reduce(&compute_ms_local, nullptr, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
            MPI_Reduce(&total_iters_local, nullptr, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return 0;
}
