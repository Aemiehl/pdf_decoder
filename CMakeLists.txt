cmake_minimum_required(VERSION 3.16)
project(png_decoder LANGUAGES CXX)
 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
 
# Standard-Build-Typ, falls keiner angegeben ist
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug)
endif()
 
# Nur main.cpp, weil Huffman.cpp per #include eingebunden wird
add_executable(decoder src/main.cpp src/Inflate.cpp src/Huffman.cpp src/LZ77.cpp src/BitReader.cpp)
 
# Pfad zum data-Ordner, damit das Programm von überall aus die Bilder findet
target_compile_definitions(decoder PRIVATE DATA_DIR="${CMAKE_SOURCE_DIR}/data")
 
if(MSVC)
    target_compile_options(decoder PRIVATE /W4)
else()
    target_compile_options(decoder PRIVATE -Wall -Wextra)
endif()
 
