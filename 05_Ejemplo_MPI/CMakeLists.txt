cmake_minimum_required(VERSION 3.16)

project(MPI_Ejemplo)


find_package(fmt CONFIG REQUIRED)

#--config mpi
set(MPI_ROOT "c:/oneAPI/mpi/latest")
set(MPI_INCLUDE "${MPI_ROOT}/include")
set(MPI_LIB "${MPI_ROOT}/lib")

include_directories(   
     ${MPI_INCLUDE}
     )


add_executable(PruebaColectivoImagen-colab
     PruebaColectivoImagen.cpp
)

target_link_libraries(PruebaColectivoImagen-colab PRIVATE 
fmt::fmt 
${MPI_LIB}/impi.lib
)