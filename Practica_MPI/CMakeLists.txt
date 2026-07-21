cmake_minimum_required(VERSION 3.16)

project(practicaMPI)


find_package(fmt CONFIG REQUIRED)

#--config mpi
set(MPI_ROOT "c:/oneAPI/mpi/latest")
set(MPI_INCLUDE "${MPI_ROOT}/include")
set(MPI_LIB "${MPI_ROOT}/lib")

include_directories(   
     ${MPI_INCLUDE}
     )


add_executable(practicaMPI
     main.cpp
     matriz_vector.cpp
)

target_link_libraries(practicaMPI PRIVATE 
fmt::fmt 
${MPI_LIB}/impi.lib
)