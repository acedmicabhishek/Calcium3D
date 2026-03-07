project('calcium3d_player', 'cpp', 'c',
  version : '0.1',
  default_options : ['warning_level=0', 'cpp_std=c++17', 'buildtype=release'])

cpp = meson.get_compiler('cpp')

# Dependencies
glfw_dep = dependency('glfw3', static : true, required : false)
if not glfw_dep.found()
  glfw_dep = dependency('glfw3', required : true)
endif

gl_dep = dependency('gl')
glew_dep = dependency('glew')
dl_dep = cpp.find_library('dl', required : false)
m_dep = cpp.find_library('m', required : false)
thread_dep = dependency('threads')
glm_dep = dependency('glm', required : false)
json_dep = dependency('nlohmann_json', required : false)
avutil_dep = dependency('libavutil', required : true)
avcodec_dep = dependency('libavcodec', required : true)
avformat_dep = dependency('libavformat', required : true)
swscale_dep = dependency('libswscale', required : true)

# Root include directories
inc_dirs = include_directories(
  'include',
  'include/tinyobj',
  'include/stb',
  'include/ufbx',
  'src',
  'src/Core',
  'src/Renderer',
  'src/Scene',
  'src/Environment',
  'src/ModelImport',
  'src/AudioEngine',
  'src/UI',
  'src/UI/Screens',
  'imgui',
  'imgui/backends'
)

sources = files(
  'src/Core/Application.cpp',
  'src/Core/GPUManager.cpp',
  'src/Core/State.cpp',
  'src/Core/RuntimeApplication.cpp',
  'src/Core/Console.cpp',
  'src/Core/InputManager.cpp',
  'src/Core/Logger.cpp',
  'src/Core/ResourceManager.cpp',
  'src/Core/main.cpp',
  'src/Environment/2dCloud.cpp',
  'src/Environment/VolumetricCloud.cpp',
  'src/Renderer/Camera.cpp',
  'src/Renderer/EBO.cpp',
  'src/Renderer/Mesh.cpp',
  'src/Renderer/Model.cpp',
  'src/Renderer/Passes/GeometryPass.cpp',
  'src/Renderer/Passes/SSRPass.cpp',
  'src/Renderer/Passes/ShadowPass.cpp',
  'src/Renderer/Passes/SkyPass.cpp',
  'src/Renderer/Passes/TransparencyPass.cpp',
  'src/Renderer/Passes/UnderwaterPass.cpp',
  'src/Renderer/Pipelines/StandardPipeline.cpp',
  'src/Renderer/RenderPipeline.cpp',
  'src/AudioEngine/audioEngine.cpp',
  'src/Physics/PhysicsEngine.cpp',
  'src/Physics/HitboxGraphics.cpp',
  'src/Renderer/Renderer.cpp',
  'src/Renderer/Shader.cpp',
  'src/Renderer/Texture.cpp',
  'src/Renderer/VideoPlayer.cpp',
  'src/Renderer/VAO.cpp',
  'src/Renderer/VBO.cpp',
  'src/Scene/ObjectFactory.cpp',
  'src/Scene/Scene.cpp',
  'src/Scene/SceneIO.cpp',
  'src/Scene/SceneManager.cpp',
  'src/Scene/Behavior.cpp',
  'src/Scene/BehaviorRegistry.cpp',
  'src/Core/ThreadManager.cpp',
  'src/Game/game_code.cpp',
  'src/UI/UIManager.cpp',
  'src/UI/UICreationEngine.cpp',
  'src/UI/Screens/StartScreen.cpp',
  'src/UI/Screens/GameplayScreen.cpp',
  'src/UI/Screens/GameScreen.cpp',
  'src/UI/Screens/FallbackScreen.cpp',
  'src/ModelImport/ModelImporter.cpp',
  'src/glad.c',
  'fs/filesystem.cpp',
  'src/Core/stb_impl.cpp',
  'src/Renderer/tinyobj_impl.cpp',
  'include/ufbx/ufbx_impl.c',
  'imgui/imgui.cpp',
  'imgui/imgui_draw.cpp',
  'imgui/imgui_widgets.cpp',
  'imgui/imgui_tables.cpp',
  'imgui/backends/imgui_impl_glfw.cpp',
  'imgui/backends/imgui_impl_opengl3.cpp'
)

executable('calcium3d_player',
  sources,
  include_directories : inc_dirs,
  dependencies : [glfw_dep, gl_dep, glew_dep, dl_dep, m_dep, thread_dep, glm_dep, json_dep, avutil_dep, avcodec_dep, avformat_dep, swscale_dep],
  cpp_args: ['-DC3D_RUNTIME', '-DIMGUI_IMPL_OPENGL_LOADER_GLAD', '-DIMGUI_DEFINE_MATH_OPERATORS', '-w'],
  link_args: ['-fuse-ld=gold'],
  install : true
)
