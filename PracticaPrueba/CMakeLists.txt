cmake_minimum_required(VERSION 3.16)

project(PracticaPrueba)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -mavx2")


find_package(fmt CONFIG REQUIRED)


add_executable(PracticaPrueba
Prueba.cpp
)

target_link_libraries(PracticaPrueba PRIVATE 
fmt::fmt 

)