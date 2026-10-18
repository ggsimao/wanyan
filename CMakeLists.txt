cmake_minimum_required(VERSION 3.16)

project(wanyan VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Debug unless asked otherwise: asserts, the binding checks and the GL debug
# context are what you want while developing.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(WANYAN_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
set(WANYAN_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

set(OpenGL_GL_PREFERENCE GLVND)  # the modern libOpenGL; silences CMP0072
find_package(SDL2 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(assimp REQUIRED)

# stb is third-party code: built separately, without the project's warnings,
# so a new compiler's warning in it can't fail the build.
add_library(stb STATIC
    ${WANYAN_SOURCE_DIR}/stb/stb_image.cpp
    ${WANYAN_SOURCE_DIR}/stb/stb_image_write.cpp
)
target_include_directories(stb PRIVATE ${WANYAN_INCLUDE_DIR})
target_compile_options(stb PRIVATE -w)

# Every other .cpp under src/. CONFIGURE_DEPENDS re-checks for added or
# removed files on each build.
file(GLOB_RECURSE WANYAN_SOURCES CONFIGURE_DEPENDS ${WANYAN_SOURCE_DIR}/*.cpp)
list(FILTER WANYAN_SOURCES EXCLUDE REGEX "/stb/")

add_executable(wanyan ${WANYAN_SOURCES})
target_include_directories(wanyan PRIVATE ${WANYAN_INCLUDE_DIR})
target_link_libraries(wanyan PRIVATE stb SDL2::SDL2 OpenGL::GL GLEW::GLEW assimp::assimp)

# Standard C++17, not GCC's gnu++17 dialect. Spelled out as properties:
# with only cxx_std_17, CMake passes no flag when the compiler's default
# already satisfies it, and the extensions stay on.
set_target_properties(wanyan stb PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Lets the program find shaders/ and resources/ from any working directory.
target_compile_definitions(wanyan PRIVATE WANYAN_ROOT="${PROJECT_SOURCE_DIR}")

target_compile_options(wanyan PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
)
