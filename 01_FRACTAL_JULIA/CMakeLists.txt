cmake_minimum_required(VERSION 3.16)

project(01_FRACTAL_JULIA)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")


find_package(fmt CONFIG REQUIRED)
find_package(SFML COMPONENTS 
                    Network
                    System 
                    Graphics 
                    Window
                    Audio
                    CONFIG REQUIRED)

add_executable(01_FRACTAL_JULIA 
main.cpp
fractal_serial.cpp
palette.cpp
fractal_simd.cpp
)

target_link_libraries(01_FRACTAL_JULIA PRIVATE 
fmt::fmt 
SFML::System
SFML::Graphics
SFML::Window
SFML::Network
SFML::Audio
)