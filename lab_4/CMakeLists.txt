cmake_minimum_required(VERSION 3.10)

project(Lab4)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SFML, OpenGL and GLM
find_package(SFML 2.5 COMPONENTS window system REQUIRED)
find_package(OpenGL REQUIRED)


# Include directories for OpenGL
include_directories(${OPENGL_INCLUDE_DIRS})
include_directories(${GLM_INCLUDE_DIRS})

# Add executable
add_executable(lab_4 lab_4.cpp)

# Link libraries
target_link_libraries(lab_4 sfml-window sfml-system ${OPENGL_LIBRARIES} GLU GLEW)
