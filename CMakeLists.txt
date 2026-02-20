cmake_minimum_required(VERSION 3.10)
project(calcium3d)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

# Include directories
include_directories(include include/stb include/glm include/glad imgui imgui/backends fs include/tinyobj)

# Find required packages
find_package(OpenGL REQUIRED) #1 
find_package(PkgConfig REQUIRED) #2
pkg_check_modules(GLFW REQUIRED glfw3) #3
pkg_check_modules(GLEW REQUIRED glew) #4
find_package(nlohmann_json 3 REQUIRED) #5
# find_package(tinyobjloader REQUIRED) #6

# Set sources
set(SOURCES
    # Core
    src/Core/main.cpp
    src/Core/Application.cpp
    src/Core/RuntimeApplication.cpp
    src/Core/EditorApplication.cpp
    src/Core/InputManager.cpp
    src/Core/Logger.cpp
    src/Core/ResourceManager.cpp
    src/Core/stb_impl.cpp
    
    # Home
    src/Home/Home.cpp
    
    # Build Manager
    src/BuildManager/BuildManager.cpp
    
    # Renderer
    src/Renderer/Renderer.cpp
    src/Renderer/Shader.cpp
    src/Renderer/Camera.cpp
    src/Renderer/Texture.cpp
    src/Renderer/Mesh.cpp
    src/Renderer/Model.cpp
    src/Renderer/VAO.cpp
    src/Renderer/VBO.cpp
    src/Renderer/EBO.cpp
    src/Renderer/tinyobj_impl.cpp
    
    src/Renderer/Passes/GeometryPass.cpp
    src/Renderer/Passes/ShadowPass.cpp
    src/Renderer/Passes/SkyPass.cpp
    src/Renderer/Passes/TransparencyPass.cpp
    src/Renderer/Pipelines/StandardPipeline.cpp
    src/Renderer/RenderPipeline.cpp

    # Scene
    src/Physics/PhysicsEngine.cpp
    src/Scene/Scene.cpp
    src/Scene/ObjectFactory.cpp
    src/Scene/SceneIO.cpp
    
    # Editor
    src/Editor/Editor.cpp
    src/Editor/EditorLayer.cpp
    src/Editor/Gizmo.cpp
    
    # Environment
    src/Environment/Water.cpp
    src/Environment/2dCloud.cpp
    src/Environment/VolumetricCloud.cpp
    
    # ImGui
    src/glad.c
    fs/filesystem.cpp
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.cpp
    imgui/ImGuizmo.cpp
)

# Add include directories so we don't need relative paths
# Create executable
add_executable(calcium3d ${SOURCES})

# Add include directories so we don't need relative paths
target_include_directories(calcium3d PUBLIC
    src/Core
    src/Renderer
    src/Scene
    src/Editor
    src/Environment
    src/Home
    src/BuildManager
    src
)

# Link libraries
target_link_libraries(calcium3d
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARIES}
    ${GLEW_LIBRARIES}
    nlohmann_json::nlohmann_json
    # tinyobjloader
)

# Include pkg-config flags (in case needed)
target_include_directories(calcium3d PRIVATE
    ${GLFW_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    # ${TINYOBJLOADER_INCLUDE_DIRS}
)

target_link_directories(calcium3d PRIVATE
    ${GLFW_LIBRARY_DIRS}
    ${GLEW_LIBRARY_DIRS}
)

# Test Build Target for Standalone Runtime (No Editor/ImGui) 

set(TESTBUILD_SOURCES
    # Core
    src/Core/main.cpp
    src/Core/Application.cpp
    src/Core/RuntimeApplication.cpp
    src/Core/InputManager.cpp
    src/Core/Logger.cpp
    src/Core/ResourceManager.cpp
    src/Core/stb_impl.cpp
    
    # Renderer
    src/Renderer/Renderer.cpp
    src/Renderer/Shader.cpp
    src/Renderer/Camera.cpp
    src/Renderer/Texture.cpp
    src/Renderer/Mesh.cpp
    src/Renderer/Model.cpp
    src/Renderer/VAO.cpp
    src/Renderer/VBO.cpp
    src/Renderer/EBO.cpp
    src/Renderer/tinyobj_impl.cpp
    
    src/Renderer/Passes/GeometryPass.cpp
    src/Renderer/Passes/ShadowPass.cpp
    src/Renderer/Passes/SkyPass.cpp
    src/Renderer/Passes/TransparencyPass.cpp
    src/Renderer/Pipelines/StandardPipeline.cpp
    src/Renderer/RenderPipeline.cpp

    # Scene
    src/Physics/PhysicsEngine.cpp
    src/Scene/Scene.cpp
    src/Scene/ObjectFactory.cpp
    src/Scene/SceneIO.cpp
    
    # Environment
    src/Environment/Water.cpp
    src/Environment/2dCloud.cpp
    src/Environment/VolumetricCloud.cpp
    
    # Glad
    src/glad.c
)

add_executable(calcium3d_testbuild ${TESTBUILD_SOURCES})

target_compile_definitions(calcium3d_testbuild PUBLIC C3D_RUNTIME)

target_include_directories(calcium3d_testbuild PUBLIC
    src/Core
    src/Renderer
    src/Scene
    src/Environment
    src
)

target_link_libraries(calcium3d_testbuild
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARIES}
    ${GLEW_LIBRARIES}
    nlohmann_json::nlohmann_json
)

target_include_directories(calcium3d_testbuild PRIVATE
    ${GLFW_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
)

target_link_directories(calcium3d_testbuild PRIVATE
    ${GLFW_LIBRARY_DIRS}
    ${GLEW_LIBRARY_DIRS}
)

# # Optional: enable compiler warnings
# target_compile_options(calcium3d PRIVATE -Wall -Wextra -pedantic)

# Set optimization level
# target_compile_options(calcium3d PRIVATE -O0)
