#include <fmt/core.h>
#include <SFML/Graphics.hpp>

#include <complex>
#include "fractal_serial.h"
#include "fractal_simd.h"

#ifdef _WIN32
    #include <windows.h>
#endif

// Pamarametro img
#define ANCHO 1600
#define ALTO 900

//Parameteros
int max_iteraciones = 10;
double x_min = -1.5;
double x_max = 1.5;
double y_min = -1.0;
double y_max = 1.0;

//Complejo que almacena dobles
std::complex<double> c(-0.71, 0.27015);

//textura (uint32_t tipo de dato sin signo de 16/32/64 etc)
uint32_t* pixel_buffer = nullptr;
// uint16_t* pixel_buffer = nullptr;

enum class runtime_type{
    SERIAL_1 = 0,
    SERIAL_2,
    SIMD
};

int main(){

    runtime_type r_type = runtime_type::SERIAL_1;

    // en un solo vector puede representar todo la img ya sea por filas o columnas
    pixel_buffer = new uint32_t[ANCHO*ALTO];

    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();

    sf::RenderWindow window(sf::VideoMode({ANCHO, ALTO}), "Julia");

#ifdef _WIN32
    HWND hwnd = window.getNativeHandle();
    ShowWindow(hwnd, SW_MAXIMIZE); // Maximizar Ventana
#endif

        sf::Texture texture({ANCHO, ALTO});
        sf::Sprite sprite(texture);

        sf::Font font("arial.ttf");
        sf::Text text(font,"Julia Set",24);
        text.setFillColor(sf::Color::White);
        text.setPosition ({10,10});
        text.setStyle(sf::Text::Bold);

        std::string options = "Options: [1] Serial [2] Serial 2 [3] SIMD | Up/Down: Change iterations";
        sf::Text textOptions(font,options,20);
        textOptions.setFillColor(sf::Color::White);
        textOptions.setPosition ({10,window.getView().getSize().y -40});
        textOptions.setStyle(sf::Text::Bold);
        //FPS
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
            else if (event -> is < sf::Event::KeyReleased>() ){
                auto evt =event -> getIf <sf::Event::KeyReleased>();

                switch(evt -> scancode){
                    case sf::Keyboard::Scan::Up:
                        max_iteraciones += 10;
                        break;
                    case sf::Keyboard::Scan::Down:
                        max_iteraciones -= 10;
                        if(max_iteraciones <10) max_iteraciones = 10;
                        break;
                    case sf::Keyboard::Scan::Num1:
                        r_type = runtime_type::SERIAL_1;
                        break;
                    case sf::Keyboard::Scan::Num2:
                        r_type = runtime_type::SERIAL_2;
                        break;
                    case sf::Keyboard::Scan::Num3:
                        r_type = runtime_type::SIMD;
                        break;
                }
            }
        }

        // Dibuja Fractal dependera de la velocidad
        
        std::string mode = "";
        if(r_type == runtime_type::SERIAL_1){
            julia_serial_1(x_min,y_min,x_max,y_max, ANCHO, ALTO,pixel_buffer);
            mode = "SERIAL_1";
        } else if (r_type == runtime_type::SERIAL_2){
            julia_serial_2(x_min,y_min,x_max,y_max, ANCHO, ALTO,pixel_buffer);
            mode = "SERIAL_2";
        } else if (r_type == runtime_type::SIMD){
            julia_serial_simd(x_min,y_min,x_max,y_max, ANCHO, ALTO,pixel_buffer);
            mode = "SIMD";
        }
        
        texture.update((const uint8_t *)pixel_buffer);
        


        frames++;

        if(clock.getElapsedTime().asSeconds()>= 1.0f){
            fps = frames;
            frames = 0;
            clock.restart();
        }
        auto msg = fmt::format("julia: iteraciones; {}.fps{} Mode: {}", max_iteraciones,fps, mode);
        text.setString(msg);


        
        // Clear screen
        window.clear();
        {
            window.draw(sprite);
            window.draw(text);
            window.draw(textOptions);

        }

        // Update the window
        window.display();
    }

    delete[] pixel_buffer;
    return 0;
}
