cmake_minimum_required(VERSION 2.6)
project(Tarea2)

if (CMAKE_BUILD_TYPE STREQUAL "")
	# CMake defaults to leaving CMAKE_BUILD_TYPE empty. This screws up
	# differentiation between debug and release builds.
	set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: None (CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif ()

# Output paths.
set(CMAKE_DEBUG_POSTFIX "_d")
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/dist")

# Turn warnings.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# Find opengl
find_package(OpenGL REQUIRED)
if(NOT OPENGL_FOUND)
    message(SEND_ERROR "Failed to find OpenGL")
endif()

# Find GLEW
find_library(GLEW_LIB GLEW)
if(NOT GLEW_LIB)
    message(SEND_ERROR "Failed to find GLEW")
endif()

# Find SDL
find_package(SDL REQUIRED)
if(NOT SDL_FOUND)
    message(SEND_ERROR "Failed to find SDL")
endif()

# Add the current directory.
include_directories(.)

# Add the local include directory.
include_directories(include)

# Set output dir.
set(EXECUTABLE_OUTPUT_PATH "${Tarea2_BINARY_DIR}/dist")
set(LIBRARY_OUTPUT_PATH "${Tarea2_BINARY_DIR}/dist")

# Add the source directory.
add_subdirectory(src)

