cmake_minimum_required(VERSION 3.5)
project(findepatzer VERSION 1.0 LANGUAGES CXX)

# C++11 standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/Board.cpp
    src/Game.cpp
    src/Move.cpp
    src/PgnParser.cpp
    src/StockfishEngine.cpp
    src/BlunderAnalyzer.cpp
    src/Config.cpp
)

# Create executable
add_executable(findepatzer ${SOURCES})

# Install target
install(TARGETS findepatzer DESTINATION bin)
