cmake_minimum_required(VERSION 3.16)
project(Othello VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# Set source files
set(SOURCES
    src/main.cpp
    src/game.cpp
    src/board.cpp
    src/ai_agent.cpp
    src/console_interface.cpp
    src/gui_interface.cpp
)

set(HEADERS
    include/game.h
    include/board.h
    include/ai_agent.h
    include/console_interface.h
    include/gui_interface.h
)

# Create executable
add_executable(othello ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(othello PRIVATE include)

# Link libraries
target_link_libraries(othello sfml-graphics sfml-window sfml-system)

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(othello PRIVATE -Wall -Wextra -O2)
endif()

# Installation
install(TARGETS othello DESTINATION bin)
