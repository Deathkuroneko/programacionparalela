#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
// Parameteros
#include <complex>
#include <fmt/core.h>
#include <SFML/Graphics.hpp>
#include "fractal_mpi.h"
#include "mpi.h"
#include "arial_ttf.h"
#include "draw_text.h"
#include <minwindef.h>
#include <winbase.h>

std::string machine_name() {
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
double x_min = -1.5;
double x_max = 1.5;
double y_min = -1.0;
double y_max = 1.0;
uint32_t *pixel_buffer = nullptr;
uint32_t *texture_buffer = nullptr;
int running = 1;
int delta; // número de filas que cada proceso debe calcular, incluyendo el padding
int rows_start;
int rows_end;
int nprocs, rank;
// Pamarametro img
#define ANCHO 1600
#define ALTO 900
int padding;

void dibujar_texto(int rank)
{
    auto texto = fmt::format("RANK_{} - {}", rank, machine_name());

    if (rank == 0) {
        draw_text_to_texture((unsigned char *)texture_buffer,
                             ANCHO, ALTO,
                             texto.c_str(),
                             10, 25, 20);
    } else {
        int send_rows = rows_end - rows_start;
        draw_text_to_texture((unsigned char *)pixel_buffer,
                             ANCHO, send_rows,
                             texto.c_str(),
                             10, 25, 20);
    }
}

// Complejo que almacena dobles
std::complex<double> c(-0.71, 0.27015);

void setup_ui()
{
    texture_buffer = new uint32_t[ANCHO * ALTO];
    std::memset(texture_buffer, 0, ANCHO * ALTO * sizeof(uint32_t));
    // inicializar la ui
    // sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    sf::RenderWindow window(sf::VideoMode({ANCHO, ALTO}), "Fractal MPI");

#ifdef _WIN32
    HWND hwnd = window.getNativeHandle();
    ShowWindow(hwnd, SW_MAXIMIZE); // Maximizar Ventana
#endif

    sf::Texture texture({ANCHO, ALTO});
    texture.update((const uint8_t *)texture_buffer);
    sf::Sprite sprite(texture);
    // textos
    sf::Font font(arial_ttf::data, arial_ttf::data_len);
    sf::Text text(font, "Fractal", 24);
    text.setFillColor(sf::Color::White);
    text.setPosition({10, 10});
    text.setStyle(sf::Text::Bold);

    std::string options = "Up/Down change iterations";
    sf::Text textoptions(font, options, 20);

    textoptions.setStyle(sf::Text::Bold);
    textoptions.setPosition({10, window.getView().getSize().y - 40});
    // fps
    int frames = 0;
    int fps = 0;
    sf::Clock clock;

    while (window.isOpen())
    {
        // Process events
        while (const std::optional event = window.pollEvent())
        {

            // Close window: exit
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
                    break;
                case sf::Keyboard::Scan::Down:
                    max_iteraciones -= 10;
                    if (max_iteraciones < 10)
                        max_iteraciones = 10;
                    break;
                }
                std::memset(texture_buffer, 0, ANCHO * ALTO * sizeof(uint32_t));
            }
        }

        // notificar a los otros ransk que la app se esta cerrando
        std::vector<int> dummy = {max_iteraciones, running};
        MPI_Bcast(dummy.data(), 2, MPI_INT, 0, MPI_COMM_WORLD);

        if (running == 0)
        {
            break;
        }
        julia_mpi(x_min, y_min, x_max, y_max, ANCHO, ALTO, rows_start, rows_end, pixel_buffer);
        std::memcpy(texture_buffer, pixel_buffer, ANCHO * delta * sizeof(uint32_t));

        // recibir las porciones de la imagen de los otros procesos
        for (int i = 1; i < nprocs; i++)
        {
            int new_delta = delta;
            if (i == nprocs - 1)
            {
                new_delta = delta - padding; // el último proceso tiene menos filas debido al padding
            }
            MPI_Status status;
            MPI_Recv(
                pixel_buffer,
                ANCHO * new_delta,
                MPI_UNSIGNED,
                i, // RANK DE ENVIO
                0, // TAG
                MPI_COMM_WORLD,
                &status);
            std::memcpy(texture_buffer + i * delta * ANCHO, pixel_buffer, ANCHO * new_delta * sizeof(uint32_t));
        
        
        }
        dibujar_texto(0);
        // actualizar la textura con el nuevo buffer
        texture.update((const uint8_t *)texture_buffer);

        // Dibuja Fractal dependera de la velocidad
        frames++;

        if (clock.getElapsedTime().asSeconds() >= 1.0f)
        {
            fps = frames;
            frames = 0;
            clock.restart();
        }
        auto msg = fmt::format("julia: iteraciones; {}.fps{}", max_iteraciones, fps);
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
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    init_freetype();    

    /*
    r0: start = 0*400 = 0, end = 0*400+400 = 400
    r1: start = 1*400 = 400, end = 1*400+400 = 800
    r2: start = 2*400 = 800, end = 2*400+400 = 1200
    r3: start = 3*400 = 1200, end = 3*400+400 = 1600
    */
    delta = std::ceil(1.0 * ALTO / nprocs);
    rows_start = rank * delta;
    rows_end = rows_start + delta;
    padding = delta * nprocs - ALTO;
    // filas de padding necesarias para que cada proceso tenga el mismo número de filas

    if (rows_end > ALTO)
    {
        rows_end = ALTO;
    }

    pixel_buffer = new uint32_t[ANCHO * delta]; // cada proceso solo necesita almacenar su bloque de filas, con padding incluido
    std::memset(pixel_buffer, 0, ANCHO * delta * sizeof(uint32_t));
    fmt::print("Rank {}: rows {} to {}\n", rank, rows_start, rows_end);

    if (rank == 0)
    {
        setup_ui();
    }
    else
    {
        // dibujar el fractal
        while (true)
        {
            std::vector<int> dummy = {max_iteraciones, 0};
            MPI_Bcast(dummy.data(), 2, MPI_INT, 0, MPI_COMM_WORLD); // recibir notificación de cierre
            max_iteraciones = dummy[0];
            running = dummy[1];
            if (running == 0)
            {
                fmt::print("Rank {}: received shutdown signal, exiting...\n", rank);
                break;
            }
            julia_mpi(x_min, y_min, x_max, y_max, ANCHO, ALTO, rows_start, rows_end, pixel_buffer);
            dibujar_texto(rank);
            int send_rows = rows_end - rows_start;
            MPI_Send(
                pixel_buffer,
                ANCHO * send_rows,
                MPI_UNSIGNED,
                0, // RANK DE ENVIO
                0, // TAG
                MPI_COMM_WORLD);

            // if(rank==1)
            //{
            //    fmt::print("Rank {}: max_iteraciones = {}\n", rank, max_iteraciones);
            //   std::cout.flush();
            //}
        }
    }

    MPI_Finalize();
    return 0;
}