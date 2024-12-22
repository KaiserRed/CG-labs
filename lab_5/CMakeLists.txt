cmake_minimum_required(VERSION 3.10)

project(Lab5)

# Устанавливаем стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
find_package(OpenGL REQUIRED)

# Добавляем исполняемый файл
add_executable(lab_5 lab_5.cpp)

target_link_libraries(lab_5 PRIVATE sfml-graphics sfml-window sfml-system OpenGL::GL GLU)