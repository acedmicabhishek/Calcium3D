#include "Application.h"
#include "../Physics/HitboxGraphics.h"
#include "2dCloud.h"
#include "Logger.h"
#include "ResourceManager.h"
#include "Tools/Profiler/GpuProfiler.h"
#include "Tools/Profiler/Profiler.h"
#include "VolumetricCloud.h"
#include <chrono>
#include <imgui.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <thread>

#include "../AudioEngine/AudioEngine.h"
#include "../UI/Screens/FallbackScreen.h"
#include "../UI/UICreationEngine.h"
#include "Pipelines/StandardPipeline.h"
#include "ThreadManager.h"
#include "stb/stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Application *Application::s_Instance = nullptr;

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
  if (Application::GetInstance() &&
      Application::GetInstance()->GetWindow() == window) {
    auto app = Application::GetInstance();
    if (app->GetCamera()) {
      app->GetCamera()->UpdateSize(width, height);
    }
  }
}

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {}

void mouse_button_callback(GLFWwindow *window, int button, int action,
                           int mods) {}

Application::Application(const ApplicationSpecification &spec)
    : m_Specification(spec) {
  s_Instance = this;
}

Application::~Application() {
  Shutdown();
  s_Instance = nullptr;
}

void Application::ChangeState(int newState) {
  GameStateManager::ChangeState(newState);
}

bool Application::Init() {
  ThreadManager::Init();
  if (m_Initialized)
    return true;
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW\n";
    return false;
  }

  m_Window = glfwCreateWindow(m_Specification.Width, m_Specification.Height,
                              m_Specification.Name.c_str(), nullptr, nullptr);
  if (!m_Window) {
    std::cerr << "Failed to create GLFW window\n";
    glfwTerminate();
    return false;
  }

  glfwMakeContextCurrent(m_Window);
  glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);
  glfwSetKeyCallback(m_Window, key_callback);
  glfwSetMouseButtonCallback(m_Window, mouse_button_callback);
  glfwSwapInterval(1);

  InputManager::Init(m_Window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD\n";
    return false;
  }

  m_Scene = std::make_unique<Scene>();
  m_Camera =
      std::make_unique<Camera>(m_Specification.Width, m_Specification.Height,
                               glm::vec3(0.0f, 0.0f, 2.0f));

  glfwSetWindowUserPointer(m_Window, this);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_STENCIL_TEST);
  glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

  ResourceManager::LoadShader("default",
                              "../shaders/passes/geometry/default.vert",
                              "../shaders/passes/geometry/default.frag");
  ResourceManager::LoadShader("light", "../shaders/passes/geometry/light.vert",
                              "../shaders/passes/geometry/light.frag");
  ResourceManager::LoadShader("skybox", "../shaders/passes/sky/skybox.vert",
                              "../shaders/passes/sky/skybox.frag");
  ResourceManager::LoadShader("gradientSky",
                              "../shaders/passes/sky/gradient_sky.vert",
                              "../shaders/passes/sky/gradient_sky.frag");
  ResourceManager::LoadShader("water",
                              "../shaders/passes/transparency/water.vert",
                              "../shaders/passes/transparency/water.frag");
  ResourceManager::LoadShader("cloud2d",
                              "../shaders/passes/transparency/2dcloud.vert",
                              "../shaders/passes/transparency/2dcloud.frag");
  ResourceManager::LoadShader(
      "volumetric_cloud",
      "../shaders/passes/transparency/volumetric_cloud.vert",
      "../shaders/passes/transparency/volumetric_cloud.frag");
  ResourceManager::LoadShader("hitbox", "../shaders/editor/hitbox.vert",
                              "../shaders/editor/hitbox.frag");

  ResourceManager::LoadShader("shadow", "../shaders/passes/shadow/shadow.vert",
                              "../shaders/passes/shadow/shadow.frag");
  ResourceManager::LoadShader("point_shadow",
                              "../shaders/passes/shadow/point_shadow.vert",
                              "../shaders/passes/shadow/point_shadow.frag",
                              "../shaders/passes/shadow/point_shadow.geom");

  HitboxGraphics::Init();

  m_Cloud2D = std::make_unique<Cloud2D>();
  m_VolumetricCloud = std::make_unique<VolumetricCloud>();

  auto standardPipeline = std::make_unique<StandardPipeline>();
  standardPipeline->Init();
  m_RenderPipeline = std::move(standardPipeline);

  GameStateManager::Init();

  UICreationEngine::LoadLayout("ui_layout.json");

  GameStateManager::RegisterState<FallbackScreen>("Start Screen");
  GameStateManager::RegisterState<FallbackScreen>("Gameplay");
  GameStateManager::RegisterState<FallbackScreen>("Exit");
  GameStateManager::RegisterState<FallbackScreen>("Pause");
  GameStateManager::RegisterState<FallbackScreen>("Settings");

  GameStateManager::ChangeState((int)m_StartGameState);

  Texture defaultDiffuse = ResourceManager::LoadTexture(
      "defaultDiffuse", "../Resource/default/texture/DefaultTex.png", "diffuse",
      0);
  Texture defaultSpecular = ResourceManager::LoadTexture(
      "defaultSpecular", "../Resource/default/texture/DefaultTex.png",
      "specular", 1);

  AudioEngine::Init();

  Logger::AddLog("Application Initialized");

  m_Initialized = true;
  return true;
}

void Application::CreateMSAAFramebuffer(int samples) {
  if (m_MSAAFBO != 0) {
    glDeleteFramebuffers(1, &m_MSAAFBO);
    glDeleteTextures(1, &m_MSAAColorBuffer);
    glDeleteTextures(1, &m_MSAANormalBuffer);
    glDeleteRenderbuffers(1, &m_MSAARBO);
    m_MSAAFBO = 0;
    m_MSAAColorBuffer = 0;
    m_MSAANormalBuffer = 0;
    m_MSAARBO = 0;
  }

  m_MSAASamples = samples;
  if (m_MSAASamples > 0) {
    glGenFramebuffers(1, &m_MSAAFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_MSAAFBO);

    glGenTextures(1, &m_MSAAColorBuffer);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSAAColorBuffer);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_MSAASamples, GL_RGB,
                            m_ViewportWidth, m_ViewportHeight, GL_TRUE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D_MULTISAMPLE, m_MSAAColorBuffer, 0);

    glGenTextures(1, &m_MSAANormalBuffer);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSAANormalBuffer);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_MSAASamples,
                            GL_RGBA16F, m_ViewportWidth, m_ViewportHeight,
                            GL_TRUE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
                           GL_TEXTURE_2D_MULTISAMPLE, m_MSAANormalBuffer, 0);

    glGenRenderbuffers(1, &m_MSAARBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_MSAARBO);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_MSAASamples,
                                     GL_DEPTH24_STENCIL8, m_ViewportWidth,
                                     m_ViewportHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER, m_MSAARBO);

    unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, attachments);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      Logger::AddLog("[ERROR] MSAA Framebuffer incomplete!");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
}

void Application::ResizeMSAAFramebuffer(int width, int height) {
  m_ViewportWidth = width;
  m_ViewportHeight = height;
  if (m_MSAASamples > 0) {
    CreateMSAAFramebuffer(m_MSAASamples);
  }
}

void Application::CreateViewportFramebuffer(int width, int height) {
  if (m_ViewportFBO != 0) {
    glDeleteFramebuffers(1, &m_ViewportFBO);
    glDeleteTextures(1, &m_ViewportTexture);
    glDeleteTextures(1, &m_ViewportNormalTexture);
    glDeleteRenderbuffers(1, &m_ViewportRBO);
  }

  m_ViewportWidth = width;
  m_ViewportHeight = height;

  glGenFramebuffers(1, &m_ViewportFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, m_ViewportFBO);

  glGenTextures(1, &m_ViewportTexture);
  glBindTexture(GL_TEXTURE_2D, m_ViewportTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
               GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         m_ViewportTexture, 0);

  glGenTextures(1, &m_ViewportNormalTexture);
  glBindTexture(GL_TEXTURE_2D, m_ViewportNormalTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA,
               GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                         m_ViewportNormalTexture, 0);

  glGenRenderbuffers(1, &m_ViewportRBO);
  glBindRenderbuffer(GL_RENDERBUFFER, m_ViewportRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, m_ViewportRBO);

  unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
  glDrawBuffers(2, attachments);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    Logger::AddLog("[ERROR] Viewport Framebuffer incomplete!");

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Application::ResizeViewportFramebuffer(int width, int height) {
  m_ViewportWidth = width;
  m_ViewportHeight = height;
  CreateViewportFramebuffer(width, height);
}

void Application::Run() {
  float lastFrame = 0.0f;
  float deltaTime = 0.0f;

  while (!glfwWindowShouldClose(m_Window) && m_Running) {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    m_RenderContext.deltaTime = deltaTime;

    glfwPollEvents();
    InputManager::Update();

    GameStateManager::Update(deltaTime);

    OnUpdate(deltaTime);

    if (GameStateManager::IsState(GameState::GAMEPLAY)) {
      m_Scene->Update(deltaTime, (float)glfwGetTime());
    }

    if (m_Camera && m_Scene) {
      AudioEngine::Update(m_Scene.get(), m_Camera->Position,
                          m_Camera->Orientation, m_Camera->Up, deltaTime);
    }

    OnRender();

#ifdef C3D_RUNTIME
    int w, h;
    glfwGetFramebufferSize(m_Window, &w, &h);
    ImVec2 mainPos = ImGui::GetMainViewport()->Pos;
    GameStateManager::Render(glm::vec2(w, h), glm::vec2(mainPos.x, mainPos.y));
#endif

    PostRender();

    if (Renderer::s_LowLatencyMode) {
      glFinish();
    }

    glfwSwapBuffers(m_Window);

    if (Renderer::s_MaxFPS > 0) {
      double targetFrameTime = 1.0 / (double)Renderer::s_MaxFPS;
      double currentTime = glfwGetTime();
      double frameTime = currentTime - currentFrame;
      if (frameTime < targetFrameTime) {
        double sleepTime = targetFrameTime - frameTime;
        std::this_thread::sleep_for(std::chrono::microseconds(
            (long long)(sleepTime * 1000000.0 * 0.95))); 
        while (glfwGetTime() < currentFrame + targetFrameTime) {
          
        }
      }
    }
  }
}

void Application::OpenProject(const std::string &path) {
  m_ProjectRoot = path;
  Logger::AddLog("Opened Project: %s", path.c_str());

  std::string title = m_Specification.Name + " - " + GetProjectName();
  glfwSetWindowTitle(m_Window, title.c_str());
}

std::string Application::GetProjectName() const {
  if (m_ProjectRoot.empty())
    return "Untitled Project";

  std::filesystem::path p(m_ProjectRoot);
  return p.filename().string();
}

void Application::CreateProject(const std::string &path) {
  namespace fs = std::filesystem;
  try {
    if (!fs::exists(path)) {
      fs::create_directories(path);
      fs::create_directories(path + "/Assets");
      fs::create_directories(path + "/Scenes");
      fs::create_directories(path + "/Shaders");
    }
    OpenProject(path);
    Logger::AddLog("Created Project: %s", path.c_str());
  } catch (const std::exception &e) {
    Logger::AddLog("Error creating project: %s", e.what());
  }
}

void Application::Close() { m_Running = false; }

void Application::Shutdown() {
  if (!m_Initialized)
    return;

  ResourceManager::Clear();
  AudioEngine::Shutdown();
  glfwDestroyWindow(m_Window);
  glfwTerminate();
  ThreadManager::Shutdown();

  m_Initialized = false;
}

void Application::ProcessSceneCameras() {
  if (!m_Scene)
    return;

  GLint currentFBO;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);
  GLint currentViewport[4];
  glGetIntegerv(GL_VIEWPORT, currentViewport);

  auto &objects = m_Scene->GetObjects();
  for (auto &obj : objects) {
    if (obj.hasCamera && obj.camera.enabled) {

      if (obj.camera.fbo == 0) {
        glGenFramebuffers(1, &obj.camera.fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, obj.camera.fbo);

        glGenTextures(1, &obj.camera.renderTexture);
        glBindTexture(GL_TEXTURE_2D, obj.camera.renderTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, obj.camera.resolutionX,
                     obj.camera.resolutionY, 0, GL_RGB, GL_UNSIGNED_BYTE,
                     nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, obj.camera.renderTexture, 0);

        glGenRenderbuffers(1, &obj.camera.depthBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, obj.camera.depthBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
                              obj.camera.resolutionX, obj.camera.resolutionY);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                                  GL_RENDERBUFFER, obj.camera.depthBuffer);
      }

      Camera sceneCam(obj.camera.resolutionX, obj.camera.resolutionY,
                      obj.position);
      sceneCam.Orientation = obj.rotation * glm::vec3(0, 0, -1);
      sceneCam.Up = obj.rotation * glm::vec3(0, 1, 0);
      sceneCam.FOV = obj.camera.fov;
      sceneCam.nearPlane = obj.camera.nearPlane;
      sceneCam.farPlane = obj.camera.farPlane;

      glBindFramebuffer(GL_FRAMEBUFFER, obj.camera.fbo);
      glViewport(0, 0, obj.camera.resolutionX, obj.camera.resolutionY);
      glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      RenderContext camCtx = m_RenderContext;
      camCtx.mainFBO = obj.camera.fbo;
      camCtx.width = obj.camera.resolutionX;
      camCtx.height = obj.camera.resolutionY;
      camCtx.camera = &sceneCam;
      camCtx.renderEditorObjects = false;

      
      
      camCtx.wireframe = false;

      
      
      camCtx.cullingCamera = nullptr;
      camCtx.objCulling = false;
      camCtx.backfaceCulling = false;
      camCtx.lightCulling = false;
      camCtx.shadowCulling = false;
      camCtx.materialOptimisation = false;

      if (obj.camera.isDebugCamera &&
          obj.camera.targetCullingCameraIndex != -1) {
        
        
        camCtx.visualizeCulling = true;

        int targetIdx = obj.camera.targetCullingCameraIndex;
        if (targetIdx == -10) {
          
          camCtx.cullingCamera = m_RenderContext.camera;
        } else if (targetIdx >= 0 && targetIdx < (int)objects.size() &&
                   objects[targetIdx].hasCamera) {
          auto &targetObj = objects[targetIdx];
          if (!targetObj.camera.cam) {
            targetObj.camera.cam = std::make_shared<Camera>(
                targetObj.camera.resolutionX, targetObj.camera.resolutionY,
                targetObj.position);
          }
          targetObj.camera.cam->Position = targetObj.position;
          targetObj.camera.cam->Orientation =
              targetObj.rotation * glm::vec3(0, 0, -1);
          targetObj.camera.cam->Up = targetObj.rotation * glm::vec3(0, 1, 0);
          targetObj.camera.cam->FOV = targetObj.camera.fov;
          targetObj.camera.cam->nearPlane = targetObj.camera.nearPlane;
          targetObj.camera.cam->farPlane = targetObj.camera.farPlane;

          camCtx.cullingCamera = targetObj.camera.cam.get();
          camCtx.cullingCamera->GetProjectionMatrix();
          camCtx.cullingCamera->GetViewMatrix();
        }
      }

      {
        PROFILE_SCOPE("SceneCameras_Preview");
        GPU_PROFILE_SCOPE("SceneCameras_Preview");
        m_RenderPipeline->Execute(camCtx);
      }
    }
  }

  glBindFramebuffer(GL_FRAMEBUFFER, currentFBO);
  glViewport(currentViewport[0], currentViewport[1], currentViewport[2],
             currentViewport[3]);
}
