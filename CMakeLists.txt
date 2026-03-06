cmake_minimum_required(VERSION 3.10)
project(calcium3d)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

# Include directories
include_directories(include include/stb include/glm include/glad imgui imgui/backends fs include/tinyobj)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)
pkg_check_modules(GLEW REQUIRED glew)
pkg_check_modules(FFMPEG REQUIRED libavcodec libavformat libavutil libswscale libswresample)
find_package(nlohmann_json 3 REQUIRED)
find_package(Threads REQUIRED)

# Set sources
set(SOURCES
    # Core
    src/Core/main.cpp
    src/Core/Application.cpp
    src/Core/State.cpp
    src/Core/RuntimeApplication.cpp
    src/Core/ThreadManager.cpp
    src/Core/Console.cpp
    src/Core/EditorApplication.cpp
    src/Core/InputManager.cpp
    src/Core/Logger.cpp
    src/Core/ResourceManager.cpp
    src/Core/stb_impl.cpp
    src/Core/GameState.h
    
    # UI
    src/UI/UIManager.cpp
    src/UI/UICreationEngine.cpp
    src/UI/Screens/StartScreen.cpp
    src/UI/Screens/StartScreen.cpp
    src/UI/Screens/GameplayScreen.cpp
    src/UI/Screens/GameScreen.cpp
    src/UI/Screens/FallbackScreen.cpp
    
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
    src/Renderer/VideoPlayer.cpp
    
    src/Renderer/Passes/GeometryPass.cpp
    src/Renderer/Passes/SSRPass.cpp
    src/Renderer/Passes/ShadowPass.cpp
    src/Renderer/Passes/SkyPass.cpp
    src/Renderer/Passes/TransparencyPass.cpp
    src/Renderer/Passes/UnderwaterPass.cpp
    src/Renderer/Pipelines/StandardPipeline.cpp
    src/Renderer/RenderPipeline.cpp

    # Scene
    src/Physics/PhysicsEngine.cpp
    src/Scene/Scene.cpp
    src/Scene/ObjectFactory.cpp
    src/Scene/SceneIO.cpp
    src/Scene/SceneManager.cpp
    src/Scene/Behavior.cpp
    src/Physics/HitboxGraphics.cpp
    src/Scene/BehaviorRegistry.cpp
    
    # Editor
    src/Editor/Editor.cpp
    src/Editor/EditorLayer.cpp
    src/Editor/PlayMode.cpp
    src/Editor/Gizmo.cpp
    
    # Tools
    src/Tools/Profiler/Profiler.cpp
    src/Tools/Profiler/ProfilerUI.cpp
    src/Tools/Profiler/GpuProfiler.cpp
    
    # Model Import
    src/ModelImport/ModelImporter.cpp
    include/ufbx/ufbx_impl.c
    
    # Audio
    src/AudioEngine/audioEngine.cpp
    
    # Animation
    src/AniEngine/AniEngine.cpp
    src/AniEngine/Animation.h
    src/AniEngine/AnimatorUI.cpp
    src/AniEngine/RiggingUI.cpp
    
    # Environment
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
    src/Game/game_code.cpp
)

# Add include directories so we don't need relative paths
# Create executable
add_executable(calcium3d ${SOURCES})

set_target_properties(calcium3d PROPERTIES ENABLE_EXPORTS ON)

# Add include directories so we don't need relative paths
target_include_directories(calcium3d PUBLIC
    src/Core
    src/Renderer
    src/Scene
    src/Editor
    src/Environment
    src/Home
    src/BuildManager
    src/ModelImport
    src/AudioEngine
    src/AniEngine
    src
    include/ufbx
    src/Tools/Profiler
)

# Link libraries
target_link_libraries(calcium3d
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${FFMPEG_LIBRARIES}
    nlohmann_json::nlohmann_json
    Threads::Threads
)

# Include pkg-config flags (in case needed)
target_include_directories(calcium3d PRIVATE
    ${GLFW_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${FFMPEG_INCLUDE_DIRS}
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
    src/Core/State.cpp
    src/Core/RuntimeApplication.cpp
    src/Core/ThreadManager.cpp
    src/Core/Console.cpp
    src/Core/InputManager.cpp
    src/Core/Logger.cpp
    src/Core/ResourceManager.cpp
    src/Core/stb_impl.cpp
    
    # UI
    src/UI/UIManager.cpp
    src/UI/UICreationEngine.cpp
    src/UI/Screens/StartScreen.cpp
    src/UI/Screens/GameplayScreen.cpp
    src/UI/Screens/GameScreen.cpp
    src/UI/Screens/FallbackScreen.cpp
    
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
    src/Renderer/VideoPlayer.cpp
    
    src/Renderer/Passes/GeometryPass.cpp
    src/Renderer/Passes/SSRPass.cpp
    src/Renderer/Passes/ShadowPass.cpp
    src/Renderer/Passes/SkyPass.cpp
    src/Renderer/Passes/TransparencyPass.cpp
    src/Renderer/Passes/UnderwaterPass.cpp
    src/Renderer/Pipelines/StandardPipeline.cpp
    src/Renderer/RenderPipeline.cpp

    # Scene
    src/Physics/PhysicsEngine.cpp
    src/Scene/Scene.cpp
    src/Scene/ObjectFactory.cpp
    src/Scene/SceneIO.cpp
    src/Scene/SceneManager.cpp
    src/Scene/Behavior.cpp
    
    # Environment
    src/Environment/2dCloud.cpp
    src/Environment/VolumetricCloud.cpp
    src/Physics/HitboxGraphics.cpp
    src/Scene/BehaviorRegistry.cpp
    
    # Model Import
    src/ModelImport/ModelImporter.cpp
    include/ufbx/ufbx_impl.c
    
    # Audio
    src/AudioEngine/audioEngine.cpp
    
    # Animation
    src/AniEngine/AniEngine.cpp
    
    # Glad
    src/glad.c
    
    # ImGui (Needed for Runtime UI)
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_tables.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.cpp
    src/Game/game_code.cpp
)

add_executable(calcium3d_testbuild ${TESTBUILD_SOURCES})

target_compile_definitions(calcium3d_testbuild PUBLIC C3D_RUNTIME)

target_include_directories(calcium3d_testbuild PUBLIC
    src/Core
    src/Renderer
    src/Scene
    src/Environment
    src/AudioEngine
    src/AniEngine
    src
)

target_link_libraries(calcium3d_testbuild
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${FFMPEG_LIBRARIES}
    nlohmann_json::nlohmann_json
    Threads::Threads
)

target_include_directories(calcium3d_testbuild PRIVATE
    ${GLFW_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${FFMPEG_INCLUDE_DIRS}
)

target_link_directories(calcium3d_testbuild PRIVATE
    ${GLFW_LIBRARY_DIRS}
    ${GLEW_LIBRARY_DIRS}
)

# # Optional: enable compiler warnings
# target_compile_options(calcium3d PRIVATE -Wall -Wextra -pedantic)

# Set optimization level
# target_compile_options(calcium3d PRIVATE -O0)
