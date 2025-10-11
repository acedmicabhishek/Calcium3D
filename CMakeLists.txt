cmake_minimum_required(VERSION 3.10)
project(calcium3d)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

# Include directories
include_directories(include include/stb include/glm include/glad imgui imgui/backends)

# Find required packages
find_package(OpenGL REQUIRED) #1 
find_package(PkgConfig REQUIRED) #2
pkg_check_modules(GLFW REQUIRED glfw3) #3
pkg_check_modules(GLEW REQUIRED glew) #4
find_package(nlohmann_json 3 REQUIRED) #5
find_package(tinyobjloader REQUIRED) #6

# Set sources
set(SOURCES
    src/main.cpp
    src/Shader.cpp
    src/Camera.cpp
    src/glad.c
    src/VAO.cpp
    src/VBO.cpp
    src/EBO.cpp
    src/Texture.cpp
    src/Mesh.cpp
    src/Model.cpp
    src/Editor.cpp
    src/Gizmo.cpp
    src/ObjectFactory.cpp
    src/Logger.cpp
    src/2dCloud.cpp
    src/VolumetricCloud.cpp
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)

# Create executable
add_executable(calcium3d ${SOURCES})

# Link libraries
target_link_libraries(calcium3d
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARIES}
    ${GLEW_LIBRARIES}
    nlohmann_json::nlohmann_json
    tinyobjloader
)

# Include pkg-config flags (in case needed)
target_include_directories(calcium3d PRIVATE
    ${GLFW_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${TINYOBJLOADER_INCLUDE_DIRS}
)

target_link_directories(calcium3d PRIVATE
    ${GLFW_LIBRARY_DIRS}
    ${GLEW_LIBRARY_DIRS}
)

# # Optional: enable compiler warnings
# target_compile_options(calcium3d PRIVATE -Wall -Wextra -pedantic)

# Set optimization level
# target_compile_options(calcium3d PRIVATE -O0)
