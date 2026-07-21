cmake_minimum_required(VERSION 3.16)

project(01FRACTAL)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")


find_package(fmt CONFIG REQUIRED)

find_package(SFML COMPONENTS Audio Network System Graphics Window CONFIG REQUIRED)

add_executable(Fractal_Julia main.cpp fractal_serial.cpp palette.cpp fractal_simd.cpp)

target_link_libraries(Fractal_Julia PRIVATE 
fmt::fmt 
SFML::Network 
SFML::System 
SFML::Graphics
SFML::Window
SFML::Audio )