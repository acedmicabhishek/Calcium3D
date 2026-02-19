#include "Application.h"
#include "Logger.h"
#include "ResourceManager.h"
#include "ObjectFactory.h"
#include "Renderer.h"
#include "Editor.h"
#include "Water.h"
#include "2dCloud.h"
#include "VolumetricCloud.h"
#include "Gizmo.h" 
#include <iostream>


#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb/stb_image.h"


struct WindowData {
    Camera* camera;
    EditorLayer* editor;
    Application* app;
};

Application* Application::s_Instance = nullptr;


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    if (Application::GetInstance() && Application::GetInstance()->GetWindow() == window) {
         
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
        
    
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    
}

Application::Application(const ApplicationSpecification& spec)
    : m_Specification(spec)
{
    s_Instance = this;
    Init();
}

Application::~Application()
{
    Shutdown();
    s_Instance = nullptr;
}

void Application::Init()
{
    
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return;
    }

    
    m_Window = glfwCreateWindow(m_Specification.Width, m_Specification.Height, m_Specification.Name.c_str(), nullptr, nullptr);
    if (!m_Window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return;
    }

    
    glfwMakeContextCurrent(m_Window);
    glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);
    glfwSetKeyCallback(m_Window, key_callback);
    glfwSetMouseButtonCallback(m_Window, mouse_button_callback);
    glfwSwapInterval(1); 

    
    InputManager::Init(m_Window);
    
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return;
    }

    
    m_Scene = std::make_unique<Scene>();
    
    
    m_EditorLayer = std::make_unique<EditorLayer>();
    m_EditorLayer->Init(m_Window);
    
    
    m_Camera = std::make_unique<Camera>(m_Specification.Width, m_Specification.Height, glm::vec3(0.0f, 0.0f, 2.0f));

    
    static WindowData windowData;
    windowData.camera = m_Camera.get();
    windowData.editor = m_EditorLayer.get();
    windowData.app = this;
    glfwSetWindowUserPointer(m_Window, &windowData);

    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    
    ResourceManager::LoadShader("default", "../shaders/default.vert", "../shaders/default.frag");
    ResourceManager::LoadShader("light", "../shaders/light.vert", "../shaders/light.frag");
    ResourceManager::LoadShader("skybox", "../shaders/skybox.vert", "../shaders/skybox.frag");
    ResourceManager::LoadShader("gradientSky", "../shaders/gradient_sky.vert", "../shaders/gradient_sky.frag");
    ResourceManager::LoadShader("water", "../shaders/water.vert", "../shaders/water.frag");
    ResourceManager::LoadShader("cloud2d", "../shaders/2dcloud.vert", "../shaders/2dcloud.frag");
    ResourceManager::LoadShader("volumetric_cloud", "../shaders/volumetric_cloud.vert", "../shaders/volumetric_cloud.frag");
    ResourceManager::LoadShader("gizmo", "../shaders/gizmo.vert", "../shaders/gizmo.frag");
    
    InitSkybox();
    InitGradientSky();

    
    CreateViewportFramebuffer(m_Specification.Width, m_Specification.Height);
    
    
    m_Water = std::make_unique<Water>();
    m_Cloud2D = std::make_unique<Cloud2D>();
    m_VolumetricCloud = std::make_unique<VolumetricCloud>();
    m_Gizmo = std::make_unique<Gizmo>(); 

    
    Texture defaultDiffuse = ResourceManager::LoadTexture("defaultDiffuse", "../Resource/default/texture/DefaultTex.png", "diffuse", 0);
    Texture defaultSpecular = ResourceManager::LoadTexture("defaultSpecular", "../Resource/default/texture/DefaultTex.png", "specular", 1);

    

    
    
    
    
    
    Logger::AddLog("Application Initialized - Default Scene Loaded");
}

void Application::InitSkybox()
{
    
    float skyboxVertices[] =
    {
        
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f
    };

    unsigned int skyboxIndices[] =
    {
        
        1, 2, 6,
        6, 5, 1,
        
        0, 4, 7,
        7, 3, 0,
        
        4, 5, 6,
        6, 7, 4,
        
        0, 3, 2,
        2, 1, 0,
        
        0, 1, 5,
        5, 4, 0,
        
        3, 7, 6,
        6, 2, 3
    };

    glGenVertexArrays(1, &m_SkyboxVAO);
    glGenBuffers(1, &m_SkyboxVBO);
    unsigned int skyboxEBO;
    glGenBuffers(1, &skyboxEBO);
    glBindVertexArray(m_SkyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_SkyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    
    glGenTextures(1, &m_SkyboxTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_SkyboxTexture);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    std::string facesCubemap[6] =
    {
        "../Resource/cubemap/px.png",
        "../Resource/cubemap/nx.png",
        "../Resource/cubemap/py.png",
        "../Resource/cubemap/ny.png",
        "../Resource/cubemap/pz.png",
        "../Resource/cubemap/nz.png"
    };

    for (unsigned int i = 0; i < 6; i++)
    {
        int width, height, nrChannels;
        unsigned char* data = stbi_load(facesCubemap[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            stbi_set_flip_vertically_on_load(false);
            GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0,
                GL_RGB,
                width,
                height,
                0,
                format,
                GL_UNSIGNED_BYTE,
                data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cerr << "Failed to load cubemap texture: " << facesCubemap[i] << std::endl;
            stbi_image_free(data);
        }
    }
}

void Application::InitGradientSky() {
    
    float skyboxVertices[] =
    {
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f
    };

    unsigned int skyboxIndices[] =
    {
        1, 2, 6, 6, 5, 1,
        0, 4, 7, 7, 3, 0,
        4, 5, 6, 6, 7, 4,
        0, 3, 2, 2, 1, 0,
        0, 1, 5, 5, 4, 0,
        3, 7, 6, 6, 2, 3
    };

    glGenVertexArrays(1, &m_SkyVAO);
    glGenBuffers(1, &m_SkyVBO);
    unsigned int skyEBO;
    glGenBuffers(1, &skyEBO);
    glBindVertexArray(m_SkyVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_SkyVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void Application::RenderSkybox(const glm::mat4& view, const glm::mat4& projection)
{
    glDepthFunc(GL_LEQUAL);
    Shader& skyboxShader = ResourceManager::GetShader("skybox");
    skyboxShader.use();
    
    
    glm::mat4 viewNoTrans = glm::mat4(glm::mat3(view)); 
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.ID, "view"), 1, GL_FALSE, glm::value_ptr(viewNoTrans));
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(m_SkyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_SkyboxTexture);
    glUniform1i(glGetUniformLocation(skyboxShader.ID, "skybox"), 0);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS); 
}

void Application::RenderGradientSky(const glm::mat4& view, const glm::mat4& projection, float time) {
    glDisable(GL_DEPTH_TEST);
    Shader& gradientSkyProgram = ResourceManager::GetShader("gradientSky");
    gradientSkyProgram.use();
    
    glUniformMatrix4fv(glGetUniformLocation(gradientSkyProgram.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(gradientSkyProgram.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    
    glUniform1f(glGetUniformLocation(gradientSkyProgram.ID, "u_time"), time);
    
    
    float angle = (time - 6.0f) / 24.0f * 2.0f * glm::pi<float>();
    
    
    float dayNightCycle = sin(angle) * 0.5f + 0.5f;

    glm::vec3 dayTopColor = glm::vec3(0.5f, 0.7f, 1.0f);
    glm::vec3 nightTopColor = glm::vec3(0.0f, 0.0f, 0.1f);
    glm::vec3 dayBottomColor = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 nightBottomColor = glm::vec3(0.1f, 0.1f, 0.2f);

    glm::vec3 topColor = mix(nightTopColor, dayTopColor, dayNightCycle);
    glm::vec3 bottomColor = mix(nightBottomColor, dayBottomColor, dayNightCycle);

    glUniform3f(glGetUniformLocation(gradientSkyProgram.ID, "topColor"), topColor.x, topColor.y, topColor.z);
    glUniform3f(glGetUniformLocation(gradientSkyProgram.ID, "bottomColor"), bottomColor.x, bottomColor.y, bottomColor.z);

    glm::vec3 DynamicSunPos = glm::vec3(cos(angle) * 10.0f, sin(angle) * 10.0f, 0.0f);
    glm::vec3 DynamicMoonPos = glm::vec3(-DynamicSunPos.x, -DynamicSunPos.y, 0.0f);

    glUniform3f(glGetUniformLocation(gradientSkyProgram.ID, "DynamicSunPos"), DynamicSunPos.x, DynamicSunPos.y, DynamicSunPos.z);
    glUniform3f(glGetUniformLocation(gradientSkyProgram.ID, "DynamicMoonPos"), DynamicMoonPos.x, DynamicMoonPos.y, DynamicMoonPos.z);
    
    glUniform1f(glGetUniformLocation(gradientSkyProgram.ID, "sunBloom"), m_EditorLayer->sunBloom);
    glUniform1f(glGetUniformLocation(gradientSkyProgram.ID, "moonBloom"), m_EditorLayer->moonBloom);
    
     glUniform3f(glGetUniformLocation(gradientSkyProgram.ID, "sunColor"), 1.0f, 1.0f, 0.0f);
     glUniform3f(glGetUniformLocation(gradientSkyProgram.ID, "moonColor"), 0.8f, 0.8f, 0.9f);

    glBindVertexArray(m_SkyVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
}

void Application::CreateMSAAFramebuffer(int samples) {
    if (m_MSAAFBO != 0) {
        glDeleteFramebuffers(1, &m_MSAAFBO);
        glDeleteTextures(1, &m_MSAAColorBuffer);
        glDeleteRenderbuffers(1, &m_MSAARBO);
        m_MSAAFBO = 0;
        m_MSAAColorBuffer = 0;
        m_MSAARBO = 0;
    }

    m_MSAASamples = samples;
    if (m_MSAASamples > 0) {
        glGenFramebuffers(1, &m_MSAAFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, m_MSAAFBO);

        glGenTextures(1, &m_MSAAColorBuffer);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSAAColorBuffer);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_MSAASamples, GL_RGB, m_ViewportWidth, m_ViewportHeight, GL_TRUE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_MSAAColorBuffer, 0);

        glGenRenderbuffers(1, &m_MSAARBO);
        glBindRenderbuffer(GL_RENDERBUFFER, m_MSAARBO);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_MSAASamples, GL_DEPTH24_STENCIL8, m_ViewportWidth, m_ViewportHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_MSAARBO);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cerr << "MSAA Framebuffer error" << std::endl;
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void Application::CreateViewportFramebuffer(int width, int height) {
    
    if (m_ViewportFBO != 0) {
        glDeleteFramebuffers(1, &m_ViewportFBO);
        glDeleteTextures(1, &m_ViewportTexture);
        glDeleteRenderbuffers(1, &m_ViewportRBO);
    }

    m_ViewportWidth = width;
    m_ViewportHeight = height;

    
    glGenFramebuffers(1, &m_ViewportFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_ViewportFBO);

    
    glGenTextures(1, &m_ViewportTexture);
    glBindTexture(GL_TEXTURE_2D, m_ViewportTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ViewportTexture, 0);

    
    glGenRenderbuffers(1, &m_ViewportRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_ViewportRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_ViewportRBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Viewport Framebuffer error!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Application::Run()
{
    float lastFrame = 0.0f;
    float deltaTime = 0.0f;

    while (!glfwWindowShouldClose(m_Window) && m_Running)
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        
        if (!m_EditorLayer->isTimePaused) {
            m_EditorLayer->timeOfDay += deltaTime * m_EditorLayer->timeSpeed;
            if (m_EditorLayer->timeOfDay >= 24.0f) m_EditorLayer->timeOfDay = 0.0f;
        }

        
        glfwPollEvents();
        InputManager::Update();

        
        if (Editor::isEditMode) {
             m_Camera->Inputs(m_Window);
        }

        
        m_ShowSkybox = m_EditorLayer->showSkybox;
        m_ShowGradientSky = m_EditorLayer->showGradientSky;
        m_ShowWater = m_EditorLayer->showWater;
        m_ShowClouds = m_EditorLayer->showClouds;

        
        if (m_MSAASamples != m_EditorLayer->msaaSamples) {
            CreateMSAAFramebuffer(m_EditorLayer->msaaSamples);
        }

        
        m_EditorLayer->Begin();

        if (m_EditorLayer->viewportWidth > 0 && m_EditorLayer->viewportHeight > 0 &&
            (m_EditorLayer->viewportWidth != m_ViewportWidth || m_EditorLayer->viewportHeight != m_ViewportHeight)) {
            CreateViewportFramebuffer(m_EditorLayer->viewportWidth, m_EditorLayer->viewportHeight);
            
            if (m_EditorLayer->msaaSamples > 0) {
                CreateMSAAFramebuffer(m_EditorLayer->msaaSamples);
            }
            
            m_Camera->width = m_ViewportWidth;
            m_Camera->height = m_ViewportHeight;
        }

        
        if (m_EditorLayer->msaaSamples > 0 && m_MSAAFBO != 0) {
            glBindFramebuffer(GL_FRAMEBUFFER, m_MSAAFBO);
        } else {
            glBindFramebuffer(GL_FRAMEBUFFER, m_ViewportFBO);
        }
        glViewport(0, 0, m_ViewportWidth, m_ViewportHeight);

        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glm::mat4 view = m_Camera->GetViewMatrix();
        glm::mat4 projection = m_Camera->GetProjectionMatrix();

        
        if (m_EditorLayer->msaaSamples > 0) {
            if (m_EditorLayer->msaaWholeScene || m_EditorLayer->msaaSky) glEnable(GL_MULTISAMPLE);
            else glDisable(GL_MULTISAMPLE);
        }
        if (m_ShowGradientSky) {
            RenderGradientSky(view, projection, m_EditorLayer->timeOfDay);
        }

        
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilFunc(GL_ALWAYS, 0, 0xFF);
        glStencilMask(0xFF);

        
        Shader& defaultShader = ResourceManager::GetShader("default");
        defaultShader.use();

        
        glm::vec4 sunColor = m_EditorLayer->sunColor;
        glm::vec3 sunPos = m_EditorLayer->sunPos;
        float sunIntensity = m_EditorLayer->sunIntensity;

        
        glUniform4f(glGetUniformLocation(defaultShader.ID, "sunColor"), sunColor.x, sunColor.y, sunColor.z, sunColor.w);
        glUniform3f(glGetUniformLocation(defaultShader.ID, "sunPos"), sunPos.x, sunPos.y, sunPos.z);
        glUniform1f(glGetUniformLocation(defaultShader.ID, "sunIntensity"), sunIntensity);
        glUniform1i(glGetUniformLocation(defaultShader.ID, "sunEnabled"), m_EditorLayer->sunEnabled ? 1 : 0);
        
        
        auto& pointLights = m_Scene->GetPointLights();
        int plCount = (int)pointLights.size();
        glUniform1i(glGetUniformLocation(defaultShader.ID, "pointLightCount"), plCount);
        
        for (int i = 0; i < plCount && i < 16; ++i) {
            std::string base = "pointLights[" + std::to_string(i) + "]";
            glUniform3f(glGetUniformLocation(defaultShader.ID, (base + ".position").c_str()), 
                        pointLights[i].position.x, pointLights[i].position.y, pointLights[i].position.z);
            glUniform4f(glGetUniformLocation(defaultShader.ID, (base + ".color").c_str()), 
                        pointLights[i].color.x, pointLights[i].color.y, pointLights[i].color.z, pointLights[i].color.w);
            glUniform1f(glGetUniformLocation(defaultShader.ID, (base + ".intensity").c_str()), pointLights[i].intensity);
            glUniform1f(glGetUniformLocation(defaultShader.ID, (base + ".constant").c_str()), pointLights[i].constant);
            glUniform1f(glGetUniformLocation(defaultShader.ID, (base + ".linear").c_str()), pointLights[i].linear);
            glUniform1f(glGetUniformLocation(defaultShader.ID, (base + ".quadratic").c_str()), pointLights[i].quadratic);
        }
        
        
        float time = m_EditorLayer->timeOfDay;
        
        
        
        float angle = (time - 6.0f) / 24.0f * 2.0f * glm::pi<float>();
        
        glm::vec3 dynamicSunPos = glm::vec3(cos(angle) * 10.0f, sin(angle) * 10.0f, 0.0f);
        glm::vec3 dynamicMoonPos = glm::vec3(-dynamicSunPos.x, -dynamicSunPos.y, 0.0f); 
        
        
        glUniform3f(glGetUniformLocation(defaultShader.ID, "sunLight.direction"), dynamicSunPos.x, dynamicSunPos.y, dynamicSunPos.z);
        glUniform3f(glGetUniformLocation(defaultShader.ID, "sunLight.color"), m_EditorLayer->sunColor.r, m_EditorLayer->sunColor.g, m_EditorLayer->sunColor.b);
        float sunHeight = dynamicSunPos.y;
        float sunInt = m_EditorLayer->sunIntensity * glm::smoothstep(-2.0f, 2.0f, sunHeight);
        if (!m_EditorLayer->sunEnabled) sunInt = 0.0f;
        glUniform1f(glGetUniformLocation(defaultShader.ID, "sunLight.intensity"), sunInt);
        
        
        glUniform3f(glGetUniformLocation(defaultShader.ID, "moonLight.direction"), dynamicMoonPos.x, dynamicMoonPos.y, dynamicMoonPos.z);
        glUniform3f(glGetUniformLocation(defaultShader.ID, "moonLight.color"), m_EditorLayer->moonColor.r, m_EditorLayer->moonColor.g, m_EditorLayer->moonColor.b); 
        float moonHeight = dynamicMoonPos.y;
        float moonInt = m_EditorLayer->moonIntensity * glm::smoothstep(-2.0f, 2.0f, moonHeight);
        if (!m_EditorLayer->moonEnabled) moonInt = 0.0f;
        glUniform1f(glGetUniformLocation(defaultShader.ID, "moonLight.intensity"), moonInt);

        glUniform3f(glGetUniformLocation(defaultShader.ID, "camPos"), m_Camera->Position.x, m_Camera->Position.y, m_Camera->Position.z);

        if (m_EditorLayer->msaaSamples > 0) {
            if (m_EditorLayer->msaaWholeScene || m_EditorLayer->msaaPrimitives) glEnable(GL_MULTISAMPLE);
            else glDisable(GL_MULTISAMPLE);
        }
        Renderer::RenderScene(*m_Scene, *m_Camera, defaultShader, m_EditorLayer->globalTilingFactor);
        


        
        if (m_ShowSkybox && !m_ShowGradientSky) {
            if (m_EditorLayer->msaaSamples > 0) {
                if (m_EditorLayer->msaaWholeScene || m_EditorLayer->msaaSky) glEnable(GL_MULTISAMPLE);
                else glDisable(GL_MULTISAMPLE);
            }
            RenderSkybox(view, projection);
        }
        
        else if (m_ShowGradientSky) {
            if (m_ShowClouds) {
                if (m_EditorLayer->msaaSamples > 0) {
                    if (m_EditorLayer->msaaWholeScene || m_EditorLayer->msaaClouds) glEnable(GL_MULTISAMPLE);
                    else glDisable(GL_MULTISAMPLE);
                }
                if (m_EditorLayer->cloudMode == 0 && m_Cloud2D) {
                    
                    Shader& cloud2dShader = ResourceManager::GetShader("cloud2d");
                    
                    
                    m_Cloud2D->cloudColor = m_EditorLayer->cloudColor;
                    m_Cloud2D->cloudCover = m_EditorLayer->cloudCover;
                    m_Cloud2D->cloudSpeed = m_EditorLayer->cloudSpeed;
                    m_Cloud2D->tiling = m_EditorLayer->cloudTiling;
                    m_Cloud2D->density = m_EditorLayer->cloudDensity;
                    m_Cloud2D->cloudSize = m_EditorLayer->cloudSize;
                    m_Cloud2D->randomness = m_EditorLayer->cloudRandomness;

                    glm::mat4 cloud2dModel = glm::mat4(1.0f);
                    cloud2dModel = glm::translate(cloud2dModel, glm::vec3(0.0f, m_EditorLayer->cloud2dHeight, 0.0f));
                    cloud2dModel = glm::rotate(cloud2dModel, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                    cloud2dModel = glm::scale(cloud2dModel, glm::vec3(m_Camera->farPlane * 2.0f));
                    m_Cloud2D->Draw(cloud2dShader, *m_Camera, cloud2dModel);
                } else if (m_EditorLayer->cloudMode == 1 && m_VolumetricCloud) {
                    
                    Shader& volCloudShader = ResourceManager::GetShader("volumetric_cloud");
                    
                    
                    m_VolumetricCloud->density = m_EditorLayer->volCloudDensity;
                    m_VolumetricCloud->stepSize = m_EditorLayer->volCloudStepSize;
                    m_VolumetricCloud->cloudCover = m_EditorLayer->volCloudCover;
                    m_VolumetricCloud->speed = m_EditorLayer->volCloudSpeed;
                    m_VolumetricCloud->detail = m_EditorLayer->volCloudDetail;
                    m_VolumetricCloud->quality = m_EditorLayer->volCloudQuality;

                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    glDisable(GL_DEPTH_TEST);
                    m_VolumetricCloud->Draw(volCloudShader, *m_Camera, m_EditorLayer->cloud2dHeight, m_Camera->farPlane);
                    glEnable(GL_DEPTH_TEST);
                    glDisable(GL_BLEND);
                }
            }
        }

        
        if (m_ShowWater && m_Water) {
            if (m_EditorLayer->msaaSamples > 0) {
                if (m_EditorLayer->msaaWholeScene || m_EditorLayer->msaaWater) glEnable(GL_MULTISAMPLE);
                else glDisable(GL_MULTISAMPLE);
            }
            Shader& waterShader = ResourceManager::GetShader("water");
            
            
            m_Water->position.y = m_EditorLayer->waterHeight;
            m_Water->waveSpeed = m_EditorLayer->waveSpeed;
            m_Water->waveStrength = m_EditorLayer->waveStrength;
            m_Water->shininess = m_EditorLayer->waterShininess;
            m_Water->waterColor = m_EditorLayer->waterColor;
            m_Water->waveSystem = m_EditorLayer->waveSystem;

            m_Water->Draw(waterShader, *m_Camera, projection, currentFrame, m_Camera->Position);
        }

        
        if (m_EditorLayer->msaaSamples > 0 && m_MSAAFBO != 0) {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, m_MSAAFBO);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_ViewportFBO);
            glBlitFramebuffer(0, 0, m_ViewportWidth, m_ViewportHeight, 
                              0, 0, m_ViewportWidth, m_ViewportHeight, 
                              GL_COLOR_BUFFER_BIT, GL_NEAREST);
        }

        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        
        int winW, winH;
        glfwGetFramebufferSize(m_Window, &winW, &winH);
        glViewport(0, 0, winW, winH);
        glClearColor(0.06f, 0.06f, 0.06f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        
        m_EditorLayer->viewportTextureID = m_ViewportTexture;

        
        

        
        m_EditorLayer->Render(*m_Scene, *m_Camera, deltaTime);
        m_EditorLayer->End();

        
        glfwSwapBuffers(m_Window);
    }
}

void Application::Close()
{
    m_Running = false;
}

void Application::Shutdown()
{
    m_EditorLayer->Shutdown();
    ResourceManager::Clear();
    glfwDestroyWindow(m_Window);
    glfwTerminate();
}
