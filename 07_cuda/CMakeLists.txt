cmake_minimum_required(VERSION 3.16)

project(07_CUDA)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )
enable_language(CXX CUDA)

find_package(fmt CONFIG REQUIRED)


add_executable(07_CUDA 
main.cpp
kernel.cu
)
target_include_directories(07_CUDA PRIVATE
    ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}
)



target_link_libraries(07_CUDA PRIVATE 
fmt::fmt )