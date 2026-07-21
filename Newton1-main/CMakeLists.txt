cmake_minimum_required(VERSION 3.16)

project(Newton_Fractal)

enable_language(CXX CUDA)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2 -fopenmp"  )


find_package(fmt CONFIG REQUIRED)
find_package(SFML COMPONENTS 
                    Network
                    System 
                    Graphics 
                    Window
                    Audio
                    CONFIG REQUIRED)


                    
add_executable(Newton_Fractal 
main.cpp
fractal_Newton.cpp
fractal_NewtonOMP.cpp
fractal_newton_simd.cpp
palette.cpp
newton_gpu.cu
)


target_include_directories(Newton_Fractal PRIVATE
    ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}
)


target_link_libraries(Newton_Fractal PRIVATE 
fmt::fmt 
SFML::System
SFML::Graphics
SFML::Window
SFML::Network
SFML::Audio
)