cmake_minimum_required(VERSION 3.10)
project(Dijkstra_OpenGL)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Buscar GLFW
find_package(glfw3 REQUIRED)
find_package(GLUT REQUIRED)
find_library(GLUT_LIBRARIES glut /usr/lib/x86_64-linux-gnu/)
find_path(GLUT_INCLUDE_DIRS GL/glut.h /usr/include)

# Incluir directorios de GLAD y GLFW
include_directories(include)

# Agregar los archivos fuente
add_executable(dijkstra_opengl main.cpp src/glad.c
include/glad.h
)

# Vincular bibliotecas necesarias
target_link_libraries(dijkstra_opengl ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} glfw)

