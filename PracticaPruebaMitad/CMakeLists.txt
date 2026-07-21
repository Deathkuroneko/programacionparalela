cmake_minimum_required(VERSION 3.16)

project(PracticaPruebaMitad)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -mavx2")


find_package(fmt CONFIG REQUIRED)
find_package(SFML COMPONENTS 
                    Network
                    System 
                    Graphics 
                    Window
                    Audio
                    CONFIG REQUIRED)

add_executable(PracticaPruebaMitad
escala2.cpp
)

target_link_libraries(PracticaPruebaMitad
fmt::fmt 
SFML::System
SFML::Graphics
SFML::Window
SFML::Network
SFML::Audio
)