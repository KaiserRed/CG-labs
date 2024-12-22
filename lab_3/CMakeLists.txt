cmake_minimum_required(VERSION 3.10)

project(CubeRotation)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SFML
find_package(SFML 2.5 COMPONENTS window system REQUIRED)
find_package(OpenGL REQUIRED)

# Include directories
include_directories(${OPENGL_INCLUDE_DIRS})

# Add executable
add_executable(lab_3 lab_3.cpp)

# Link libraries
target_link_libraries(lab_3 sfml-window sfml-system ${OPENGL_LIBRARIES} GLU)