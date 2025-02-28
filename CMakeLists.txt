cmake_minimum_required(VERSION 3.27)
project(safe LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_custom_target(copy_compile_commands ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_BINARY_DIR}/compile_commands.json
    ${CMAKE_SOURCE_DIR}/compile_commands.json
    DEPENDS ${CMAKE_BINARY_DIR}/compile_commands.json)


# Create the executable target
add_executable(${PROJECT_NAME} src/main.cpp)

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

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
message("Building Debug")
list(APPEND COMPILER_BUILD_FLAGS -g -Og)

else()
message("Building release")
list(APPEND COMPILER_BUILD_FLAGS -O2)

endif()

target_compile_options(${PROJECT_NAME} PRIVATE ${COMPILER_BUILD_FLAGS})

# Link against ZLIB
# target_link_libraries(${PROJECT_NAME} PRIVATE ZLIB::ZLIB)