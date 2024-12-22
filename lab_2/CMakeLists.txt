cmake_minimum_required(VERSION 3.10)

project(Lab2_SFML_3DSphere)

# Устанавливаем стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
find_package(OpenGL REQUIRED)

# Добавляем исполняемый файл
add_executable(lab_2 lab_2.cpp)

target_link_libraries(lab_2 PRIVATE sfml-graphics sfml-window sfml-system OpenGL::GL GLU)