cmake_minimum_required(VERSION 3.10)
project(calcium3d)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include include/stb include/glm include/glad)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)
pkg_check_modules(GLEW REQUIRED glew)

# Set sources
set(SOURCES
    src/main.cpp
    src/Shader.cpp
    src/Camera.cpp
    src/glad.c  # Include glad source file
    src/VAO.cpp
    src/VBO.cpp
    src/EBO.cpp
    src/Texture.cpp
)

# Create executable
add_executable(calcium3d ${SOURCES})

# Link libraries
target_link_libraries(calcium3d
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARIES}
    ${GLEW_LIBRARIES}
)

# Include pkg-config flags (in case needed)
target_include_directories(calcium3d PRIVATE
    ${GLFW_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
)

target_link_directories(calcium3d PRIVATE
    ${GLFW_LIBRARY_DIRS}
    ${GLEW_LIBRARY_DIRS}
)

# # Optional: enable compiler warnings
# target_compile_options(calcium3d PRIVATE -Wall -Wextra -pedantic)
