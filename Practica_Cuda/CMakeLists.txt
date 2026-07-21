cmake_minimum_required(VERSION 3.16)

project(FractalCuda)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )
enable_language(CXX CUDA)

find_package(fmt CONFIG REQUIRED)
find_package(SFML COMPONENTS 
                    Network
                    System 
                    Graphics 
                    Window
                    Audio
                    CONFIG REQUIRED)

add_executable(FractalCuda 
main.cpp
kernel.cu
)
target_include_directories(FractalCuda PRIVATE
    ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}
)



target_link_libraries(FractalCuda PRIVATE 
fmt::fmt 
SFML::System
SFML::Graphics
SFML::Window
SFML::Network
SFML::Audio)