cmake_minimum_required(VERSION 3.23)
project(libmuslim C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)

file(GLOB SOURCES "src/*.c")

# Build shared library (for Android and general use)
add_library(muslim SHARED ${SOURCES})

target_include_directories(muslim PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(muslim PRIVATE m)

# Set library output directory
set_target_properties(muslim PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# Build executable (for desktop/CLI use)
add_executable(libmuslim main.c)

target_include_directories(libmuslim PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(libmuslim PRIVATE muslim)
