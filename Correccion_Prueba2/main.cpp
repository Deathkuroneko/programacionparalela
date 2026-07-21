#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
#include <cmath>
#include <cstring>
#include <vector>
#include <fmt/core.h>
#include <SFML/Graphics.hpp>
#include "fractal_mpi_burningShip.h"
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

int max_iteraciones = 10;
double x_min = -2.2;
double x_max = 1.2;
double y_min = -1.5;
double y_max = 1.5;

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

    sf::RenderWindow window(sf::VideoMode({ANCHO, ALTO}), "Fractal Burning Ship - MPI");

#ifdef _WIN32
    HWND hwnd = window.getNativeHandle();
    ShowWindow(hwnd, SW_MAXIMIZE);
#endif

    sf::Texture texture({ANCHO, ALTO});
    texture.update((const uint8_t *)texture_buffer);
    sf::Sprite sprite(texture);

    sf::Font font(arial_ttf::data, arial_ttf::data_len);

    sf::Text text(font, "Burning Ship", 18);
    text.setFillColor(sf::Color::White);
    text.setPosition({10, 10});
    text.setStyle(sf::Text::Bold);

    std::string options = "Up/Down: cambiar max_iter   Esc: salir";
    sf::Text textoptions(font, options, 20);
    textoptions.setFillColor(sf::Color::White);
    textoptions.setStyle(sf::Text::Bold);
    textoptions.setPosition({10, window.getView().getSize().y - 40});

    // Buffer donde rank 0 recolecta el histograma de todos los ranks
    std::vector<long> gathered(static_cast<size_t>(nprocs) * BURNING_SHIP_NBINS);

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
                    if (max_iteraciones > 100)
                        max_iteraciones = 100;
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
        long local_hist[BURNING_SHIP_NBINS];
        burning_ship_mpi(x_min, y_min, x_max, y_max, ANCHO, ALTO, rows_start, rows_end,
                          pixel_buffer, local_hist);
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

        //MPI_Gather del histograma local de cada rank
        MPI_Gather(local_hist, BURNING_SHIP_NBINS, MPI_LONG,
                   gathered.data(), BURNING_SHIP_NBINS, MPI_LONG,
                   0, MPI_COMM_WORLD);

        // Sumar por bin lo recibido de todos los ranks
        long global_hist[BURNING_SHIP_NBINS] = {0};
        for (int p = 0; p < nprocs; p++)
        {
            for (int b = 0; b < BURNING_SHIP_NBINS; b++)
            {
                global_hist[b] += gathered[p * BURNING_SHIP_NBINS + b];
            }
        }

        texture.update((const uint8_t *)texture_buffer);

        frames++;
        if (clock.getElapsedTime().asSeconds() >= 1.0f)
        {
            fps = frames;
            frames = 0;
            clock.restart();
        }

        std::string hist_str = "[";
        for (int b = 0; b < BURNING_SHIP_NBINS; b++)
        {
            hist_str += fmt::format("{}{}", global_hist[b], (b < BURNING_SHIP_NBINS - 1) ? "," : "");
        }
        hist_str += "]";

        auto msg = fmt::format(
            "RANKs: {} | max_iter: {} | dominio: x[{:.4f},{:.4f}] y[{:.4f},{:.4f}]\n"
            "hist: {}\n"
            "fps: {} | Maquina: {} ",
            nprocs, max_iteraciones, x_min, x_max, y_min, y_max, hist_str, fps, machine_name());
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

            long local_hist[BURNING_SHIP_NBINS];
            burning_ship_mpi(x_min, y_min, x_max, y_max, ANCHO, ALTO, rows_start, rows_end,
                              pixel_buffer, local_hist);

            int send_rows = rows_end - rows_start;
            MPI_Send(
                pixel_buffer,
                ANCHO * send_rows,
                MPI_UNSIGNED,
                0,
                0,
                MPI_COMM_WORLD);

            // Enviar el histograma local al rank 0 para que lo recolecte con MPI_Gather
            MPI_Gather(local_hist, BURNING_SHIP_NBINS, MPI_LONG,
                       nullptr, BURNING_SHIP_NBINS, MPI_LONG,
                       0, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return 0;
}
