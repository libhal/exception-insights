cmake_minimum_required(VERSION 3.27)
project(safe LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)  # Ensure only standard C++ features

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(ctre REQUIRED)
find_package(libelf REQUIRED)

# Find package dependancies
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/build/Debug/generators") 

#Linking Libraries
add_executable(${PROJECT_NAME} src/main.cpp src/elf_parser.cpp src/gcc_parse.cpp)

#Linking Libraries
target_link_libraries(${PROJECT_NAME} PUBLIC ctre::ctre libelf::libelf)

# Add include directories
target_include_directories(${PROJECT_NAME} PUBLIC include/)

# Add compile options
list(APPEND COMPILER_BUILD_FLAGS   
-Werror
-Wall
-Wextra
-Wshadow
-ffunction-sections
-fdata-sections
$<$<COMPILE_LANGUAGE:CXX>:-fexceptions -frtti>)

if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
message("Building release")
list(APPEND COMPILER_BUILD_FLAGS -O2)

else()
message("Building Debug")
list(APPEND COMPILER_BUILD_FLAGS -g -Og)

endif()

target_compile_options(${PROJECT_NAME} PRIVATE ${COMPILER_BUILD_FLAGS})

libhal_unit_test(SOURCES
    tests/main.test.cpp
    # tests/testing.test.cpp

    src/elf_parser.cpp

    PACKAGES
    tl-function-ref

    LINK_LIBRARIES
    tl::function-ref
    libelf::libelf
)
libhal_unit_test(SOURCES
    tests/main.test.cpp
    # tests/testing.test.cpp

    src/elf_parser.cpp

    PACKAGES
    tl-function-ref

    LINK_LIBRARIES
    tl::function-ref
    libelf::libelf
)