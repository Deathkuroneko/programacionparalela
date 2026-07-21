cmake_minimum_required(VERSION 3.16)

project(EjemploImg)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2 -fopenmp")


find_package(fmt CONFIG REQUIRED)

find_package(SFML COMPONENTS Audio Network System Graphics Window CONFIG REQUIRED)

add_executable(ejemploImg
main.cpp
)

target_link_libraries(ejemploImg PRIVATE 
fmt::fmt 
SFML::Network 
SFML::System 
SFML::Graphics
SFML::Window
SFML::Audio 
)