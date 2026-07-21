cmake_minimum_required(VERSION 3.16)

project(06_FRACTALMPI)



find_package(fmt CONFIG REQUIRED)
find_package(SFML COMPONENTS 
                    Network
                    System 
                    Graphics 
                    Window
                    Audio
                    CONFIG REQUIRED)
find_package(Freetype REQUIRED)
#--config mpi
set(MPI_ROOT "c:/oneAPI/mpi/latest")
set(MPI_INCLUDE "${MPI_ROOT}/include")
set(MPI_LIB "${MPI_ROOT}/lib")

include_directories(   
     ${MPI_INCLUDE}
     )


add_executable(fractalmpi
main.cpp
fractal_mpi.cpp
palette.cpp
draw_text.cpp
)

target_link_libraries(fractalmpi PRIVATE 
fmt::fmt 
SFML::System
SFML::Graphics
SFML::Window
SFML::Network
SFML::Audio
Freetype::Freetype
${MPI_LIB}/impi.lib
)