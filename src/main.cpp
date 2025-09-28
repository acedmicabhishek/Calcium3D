#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
// test
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include "Camera.h"
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"
#include "Texture.h"
#include "Mesh.h"
#include "Model.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "Editor.h"
#include "Gizmo.h"
#include "ObjectFactory.h"
#include "Logger.h"
#include "Cloud.h"

struct SceneObject {
    Mesh mesh;
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    glm::vec3 scale = glm::vec3(1.0f);

    SceneObject(Mesh& m) : mesh(m) {}
};

struct WindowData {
    Camera* camera;
    unsigned int framebufferTexture;
    unsigned int RBO;
    Model* plane;
    std::vector<SceneObject>* cubes;
    int* selectedMesh;
    int* selectedCube;
    bool* isLightSelected;
    Mesh* lightMesh;
    glm::vec3* lightPos;
    int msaaSamples;
    unsigned int msaaFBO;
    unsigned int msaaColorBuffer;
    unsigned int msaaRBO;
    int currentMsaaIndex;
    // Sun (Global Light) variables
    bool* isSunSelected;
    Mesh* sunMesh;
    glm::vec3* sunPos;
    glm::vec4* sunColor;
    float* sunIntensity;
    float* farPlane;
};
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

// Function to update sun uniforms in all shaders
void updateSunUniforms(Shader& shaderProgram, Shader& sunProgram, const glm::vec4& sunColor, const glm::vec3& sunPos, float sunIntensity) {
    shaderProgram.use();
    glUniform4f(glGetUniformLocation(shaderProgram.ID, "sunColor"), sunColor.x, sunColor.y, sunColor.z, sunColor.w);
    glUniform3f(glGetUniformLocation(shaderProgram.ID, "sunPos"), sunPos.x, sunPos.y, sunPos.z);
    glUniform1f(glGetUniformLocation(shaderProgram.ID, "sunIntensity"), sunIntensity);
    
    sunProgram.use();
    glUniform4f(glGetUniformLocation(sunProgram.ID, "sunColor"), sunColor.x, sunColor.y, sunColor.z, sunColor.w);
    glUniform1f(glGetUniformLocation(sunProgram.ID, "sunIntensity"), sunIntensity);
    
    shaderProgram.use();
}

// Function to update gradient sky uniforms
void updateGradientSkyUniforms(Shader& gradientSkyProgram, const glm::vec3& sunPos, const glm::vec3& moonPos, const glm::vec4& sunColor, const glm::vec4& moonColor, float sunBloom, float moonBloom) {
    gradientSkyProgram.use();
    glUniform3f(glGetUniformLocation(gradientSkyProgram.ID, "DynamicSunPos"), sunPos.x, sunPos.y, sunPos.z);
    glUniform3f(glGetUniformLocation(gradientSkyProgram.ID, "DynamicMoonPos"), moonPos.x, moonPos.y, moonPos.z);
    glUniform3f(glGetUniformLocation(gradientSkyProgram.ID, "sunColor"), sunColor.x, sunColor.y, sunColor.z);
    glUniform3f(glGetUniformLocation(gradientSkyProgram.ID, "moonColor"), moonColor.x, moonColor.y, moonColor.z);
    glUniform1f(glGetUniformLocation(gradientSkyProgram.ID, "sunBloom"), sunBloom);
    glUniform1f(glGetUniformLocation(gradientSkyProgram.ID, "moonBloom"), moonBloom);
}

int main() {
    // Bloom variables
    float sunBloom = 0.03f;
    float moonBloom = 0.02f;

	// Initialize GLFW
	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(800, 600, "Calcium3D", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    // Tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
   
    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Triangle vertices with texture coordinates
    // Vertices coordinates
    // Vertices coordinates
 Vertex vertices[] =
 { //               COORDINATES           /            COLORS          /           NORMALS         /       TEXTURE COORDINATES    //
  Vertex{glm::vec3(-1.0f, 0.0f,  1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f)},
  Vertex{glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f)},
  Vertex{glm::vec3( 1.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f)},
  Vertex{glm::vec3( 1.0f, 0.0f,  1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)}
 };

 // Indices for vertices order
 GLuint indices[] =
 {
  0, 1, 2,
  0, 2, 3
 };

 Vertex lightVertices[] =
 { //     COORDINATES     //
  Vertex{glm::vec3(-0.1f, -0.1f,  0.1f)},
  Vertex{glm::vec3(-0.1f, -0.1f, -0.1f)},
  Vertex{glm::vec3(0.1f, -0.1f, -0.1f)},
  Vertex{glm::vec3(0.1f, -0.1f,  0.1f)},
  Vertex{glm::vec3(-0.1f,  0.1f,  0.1f)},
  Vertex{glm::vec3(-0.1f,  0.1f, -0.1f)},
  Vertex{glm::vec3(0.1f,  0.1f, -0.1f)},
  Vertex{glm::vec3(0.1f,  0.1f,  0.1f)}
 };

    GLuint lightIndices[] =
    {
        0, 1, 2,
        0, 2, 3,
        0, 4, 7,
        0, 7, 3,
        3, 7, 6,
        3, 6, 2,
        2, 6, 5,
        2, 5, 1,
        1, 5, 4,
        1, 4, 0,
        4, 5, 6,
        4, 6, 7
    };

    // Generates Shader object using shaders default.vert and default.frag
    Shader shaderProgram("../shaders/default.vert", "../shaders/default.frag");
    Shader framebufferProgram("../shaders/framebuffer.vert", "../shaders/framebuffer.frag");
    Shader gizmoProgram("../shaders/gizmo.vert", "../shaders/gizmo.frag");
    Shader outlineProgram("../shaders/outline.vert", "../shaders/outline.frag");
    Shader highlightProgram("../shaders/highlight.vert", "../shaders/highlight.frag");
    Shader selectionProgram("../shaders/selection.vert", "../shaders/selection.frag");
    Shader sunProgram("../shaders/sun.vert", "../shaders/sun.frag");
    Shader horizonProgram("../shaders/horizon.vert", "../shaders/horizon.frag");
    Shader gradientSkyProgram("../shaders/gradient_sky.vert", "../shaders/gradient_sky.frag");
    Shader cloudProgram("../shaders/cloud.vert", "../shaders/cloud.frag");
    std::vector <Vertex> verts(vertices, vertices + sizeof(vertices) / sizeof(Vertex));
    std::vector <GLuint> ind(indices, indices + sizeof(indices) / sizeof(GLuint));
    Cloud cloud("../Resource/noise/cloudnoise.jpg");
   
   
    // Shader for light cube
    Shader lightShader("../shaders/light.vert", "../shaders/light.frag");
    Shader skyboxShader("../shaders/skybox.vert", "../shaders/skybox.frag");
    std::vector <Vertex> lightVerts(lightVertices, lightVertices + sizeof(lightVertices) / sizeof(Vertex));
    std::vector <GLuint> lightInd(lightIndices, lightIndices + sizeof(lightIndices) / sizeof(GLuint));
   
   
   
    glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    glm::vec3 lightPos = glm::vec3(0.5f, 0.5f, 0.5f);
    glm::mat4 lightModel = glm::mat4(1.0f);
    lightModel = glm::translate(lightModel, lightPos);

    // Sun (Global Light) variables
    glm::vec4 sunColor = glm::vec4(1.0f, 0.95f, 0.8f, 1.0f); // Warm sunlight color
    glm::vec3 sunPos = glm::vec3(10.0f, 15.0f, 5.0f); // Position the sun high and far
    float sunIntensity = 2.0f; // Higher intensity for better lighting
    glm::mat4 sunModel = glm::mat4(1.0f);
    sunModel = glm::translate(sunModel, sunPos);
    sunModel = glm::scale(sunModel, glm::vec3(2.0f)); // Make sun larger

    glm::vec3 pyramidPos = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::mat4 pyramidModel = glm::mat4(1.0f);
    pyramidModel = glm::translate(pyramidModel, pyramidPos);


    lightShader.use();
    glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(lightModel));
    glUniform4f(glGetUniformLocation(lightShader.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
    shaderProgram.use();
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "model"), 1, GL_FALSE, glm::value_ptr(pyramidModel));
    glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
    glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);


    
    // Set sun uniforms for all shaders
    shaderProgram.use();
    glUniform4f(glGetUniformLocation(shaderProgram.ID, "sunColor"), sunColor.x, sunColor.y, sunColor.z, sunColor.w);
    glUniform3f(glGetUniformLocation(shaderProgram.ID, "sunPos"), sunPos.x, sunPos.y, sunPos.z);
    glUniform1f(glGetUniformLocation(shaderProgram.ID, "sunIntensity"), sunIntensity);
    
    
    shaderProgram.use();


    // Textures
    Texture textures[]
    {
    	Texture("../Resource/default/texture/DefaultTex.png", "diffuse", 0),
    	Texture("../Resource/default/texture/DefaultTex.png", "specular", 1)
    };
    std::vector <Texture> tex(textures, textures + sizeof(textures) / sizeof(Texture));
    Mesh floor(verts, ind, tex);
 Model* plane = nullptr;
 Mesh* light = nullptr;
 Mesh* sun = nullptr;
     Gizmo gizmo;
    gizmo.SetMode(Gizmo::TRANSLATE); // Set default mode
 int selectedMesh = -1;
 int selectedCube = -1;
     bool isLightSelected = false;
    bool isSunSelected = false;
    std::vector<SceneObject> cubes;
    
    // Global texture tiling factor
    float globalTilingFactor = 1.0f;

    // Enables the Depth Buffer
    glEnable(GL_DEPTH_TEST);

    // Creates camera object
    Camera camera(800, 600, glm::vec3(0.0f, 0.0f, 2.0f));
    unsigned int framebufferTexture;
    unsigned int RBO;
    WindowData data;
    data.camera = &camera;
    data.plane = plane;
    data.cubes = &cubes;
    data.selectedMesh = &selectedMesh;
    data.selectedCube = &selectedCube;
    data.isLightSelected = &isLightSelected;
    data.lightMesh = light;
    data.lightPos = &lightPos;
    data.msaaSamples = 0;
    data.msaaFBO = 0;
    data.msaaColorBuffer = 0;
    data.msaaColorBuffer = 0;
    data.msaaRBO = 0;
    data.currentMsaaIndex = 0;
    data.isSunSelected = &isSunSelected;
    data.sunMesh = sun;
    data.sunPos = &sunPos;
    data.sunColor = &sunColor;
    data.sunIntensity = &sunIntensity;
    data.farPlane = &camera.farPlane;
    
    // Create initial sun mesh
    Mesh sunSphere = ObjectFactory::createSphere(32, 16);
    sun = new Mesh(sunSphere);
    data.sunMesh = sun;
    
    glfwSetWindowUserPointer(window, &data);
   
    float rectangleVertices[] =
    {
    	// Coords    // texCoords
    	 1.0f, -1.0f,  1.0f, 0.0f,
    	-1.0f, -1.0f,  0.0f, 0.0f,
    	-1.0f,  1.0f,  0.0f, 1.0f,
   
    	 1.0f,  1.0f,  1.0f, 1.0f,
    	 1.0f, -1.0f,  1.0f, 0.0f,
    	-1.0f,  1.0f,  0.0f, 1.0f}
    ;
   
    unsigned int rectVAO, rectVBO;
    glGenVertexArrays(1, &rectVAO);
    glGenBuffers(1, &rectVBO);
    glBindVertexArray(rectVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices), &rectangleVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
   
   
    // Create Frame Buffer Object
    unsigned int FBO;
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
   
    // Create Framebuffer Texture
    glGenTextures(1, &framebufferTexture);
    glBindTexture(GL_TEXTURE_2D, framebufferTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, camera.width, camera.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture, 0);
   
    // Create Render Buffer Object
    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, camera.width, camera.height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
   
   
    // Error checking framebuffer
    auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
    	std::cout << "Framebuffer error: " << fboStatus << std::endl;
    
    data.framebufferTexture = framebufferTexture;
    data.RBO = RBO;

    	float skyboxVertices[] =
    	{
    		//   Coordinates
    		-1.0f, -1.0f,  1.0f,//        7--------6
    		 1.0f, -1.0f,  1.0f,//       /|       /|
    		 1.0f, -1.0f, -1.0f,//      4--------5 |
    		-1.0f, -1.0f, -1.0f,//      | |      | |
    		-1.0f,  1.0f,  1.0f,//      | 3------|-2
    		 1.0f,  1.0f,  1.0f,//      |/       |/
    		 1.0f,  1.0f, -1.0f,//      0--------1
    		-1.0f,  1.0f, -1.0f
    	};
    
    	unsigned int skyboxIndices[] =
    	{
    		// Right
    		1, 2, 6,
    		6, 5, 1,
    		// Left
    		0, 4, 7,
    		7, 3, 0,
    		// Top
    		4, 5, 6,
    		6, 7, 4,
    		// Bottom
    		0, 3, 2,
    		2, 1, 0,
    		// Back
    		0, 1, 5,
    		5, 4, 0,
    		// Front
    		3, 7, 6,
    		6, 2, 3
    	};
    	// Create VAO, VBO, and EBO for the skybox
    	unsigned int skyboxVAO, skyboxVBO, skyboxEBO;
    	glGenVertexArrays(1, &skyboxVAO);
    	glGenBuffers(1, &skyboxVBO);
    	glGenBuffers(1, &skyboxEBO);
    	glBindVertexArray(skyboxVAO);
    	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
    	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
    	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    	glEnableVertexAttribArray(0);
    	glBindBuffer(GL_ARRAY_BUFFER, 0);
    	glBindVertexArray(0);
    	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    	// Cubemap texture
    unsigned int cubemapTexture;
    glGenTextures(1, &cubemapTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
 
    // Horizon line vertices
    float horizonVertices[] = {
        -1.0f, 0.0f, 0.0f,
         1.0f, 0.0f, 0.0f
    };

    unsigned int horizonVAO, horizonVBO;
    glGenVertexArrays(1, &horizonVAO);
    glGenBuffers(1, &horizonVBO);
    glBindVertexArray(horizonVAO);
    glBindBuffer(GL_ARRAY_BUFFER, horizonVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(horizonVertices), horizonVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    // Use skybox vertices for the gradient sky
    unsigned int skyVAO, skyVBO, skyEBO;
    glGenVertexArrays(1, &skyVAO);
    glGenBuffers(1, &skyVBO);
    glGenBuffers(1, &skyEBO);
    glBindVertexArray(skyVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    const char* msaaOptions[] = { "Off", "2x", "4x", "8x" };
    int msaaSamplesValues[] = { 0, 2, 4, 8 };
 
    auto createMsaaFramebuffer = [&](int samples) {
        WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);
        if (data->msaaFBO != 0) {
            glDeleteFramebuffers(1, &data->msaaFBO);
            glDeleteTextures(1, &data->msaaColorBuffer);
            glDeleteRenderbuffers(1, &data->msaaRBO);
            data->msaaFBO = 0;
            data->msaaColorBuffer = 0;
            data->msaaRBO = 0;
        }

        data->msaaSamples = samples;
        if (data->msaaSamples > 0) {
            glGenFramebuffers(1, &data->msaaFBO);
            glBindFramebuffer(GL_FRAMEBUFFER, data->msaaFBO);

            glGenTextures(1, &data->msaaColorBuffer);
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, data->msaaColorBuffer);
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, data->msaaSamples, GL_RGB, data->camera->width, data->camera->height, GL_TRUE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, data->msaaColorBuffer, 0);

            glGenRenderbuffers(1, &data->msaaRBO);
            glBindRenderbuffer(GL_RENDERBUFFER, data->msaaRBO);
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, data->msaaSamples, GL_DEPTH24_STENCIL8, data->camera->width, data->camera->height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, data->msaaRBO);

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                std::cout << "MSAA Framebuffer error" << std::endl;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    };

    createMsaaFramebuffer(msaaSamplesValues[data.currentMsaaIndex]);
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
    			glTexImage2D
    			(
    				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
    				0,
    				GL_RGB,
    				width,
    				height,
    				0,
    				GL_RGB,
    				GL_UNSIGNED_BYTE,
    				data
    			);
    			stbi_image_free(data);
    		}
    		else
    		{
    			std::cout << "Failed to load texture: " << facesCubemap[i] << std::endl;
    			stbi_image_free(data);
    		}
    	}
    
        bool showCubemap = true;
        bool showGradientSky = false; // New: gradient/atmosphere sky toggle
        int skyMode = 0; // 0 = Cubemap, 1 = Gradient/Atmosphere
        bool showClouds = false;
bool showLightSource = false;
bool showPlane = false;
bool showPerformance = true;

// Performance tracking variables
float deltaTime = 0.0f;
float lastFrame = 0.0f;
int frameCount = 0;
float fps = 0.0f;
float avgFrameTime = 0.0f;


    	while (!glfwWindowShouldClose(window))
    	{
            // Calculate performance metrics
            float currentFrame = glfwGetTime();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;
            
            frameCount++;
            if (frameCount % 60 == 0) { // Update FPS every 60 frames
                fps = 1.0f / deltaTime;
                avgFrameTime = deltaTime * 1000.0f; // Convert to milliseconds
            }
            
            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2(300, ImGui::GetIO().DisplaySize.y));
            ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar);
            
            // Menu bar
            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
                        cubes.clear();
                        selectedCube = -1;
                        selectedMesh = -1;
                        isLightSelected = false;
                        isSunSelected = false;
                        Logger::AddLog("Created new scene");
                    }
                    if (ImGui::MenuItem("Clear Selection", "Ctrl+D")) {
                        selectedCube = -1;
                        selectedMesh = -1;
                        isLightSelected = false;
                        isSunSelected = false;
                        Logger::AddLog("Cleared all selections");
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Exit", "Alt+F4")) {
                        glfwSetWindowShouldClose(window, true);
                    }
                    ImGui::EndMenu();
                }
                            if (ImGui::BeginMenu("View")) {
                ImGui::MenuItem("Performance Info", nullptr, &showPerformance);
                ImGui::EndMenu();
            }
                            if (ImGui::BeginMenu("Help")) {
                if (ImGui::MenuItem("About")) {
                    Logger::AddLog("a light weigth game engine");
                }
                ImGui::EndMenu();
            }
                ImGui::EndMenuBar();
            }

            if (ImGui::CollapsingHeader("Display Options")) {
                const char* skyModes[] = {"Cubemap", "Dynamic Sky"};
                ImGui::Combo("Sky Mode", &skyMode, skyModes, IM_ARRAYSIZE(skyModes));
                showCubemap = (skyMode == 0);
                showGradientSky = (skyMode == 1);
                ImGui::Checkbox("Show Clouds", &showClouds);
                if (ImGui::Checkbox("Show Local Light Source", &showLightSource)) {
                    if (showLightSource && !light) {
                        std::vector<Texture> lightTex;
                        lightTex.emplace_back("../Resource/default/texture/DefaultTex.png", "diffuse", 0);
                        lightTex.emplace_back("../Resource/default/texture/DefaultTex.png", "specular", 1);
                        light = new Mesh(lightVerts, lightInd, lightTex);
                        data.lightMesh = light;
                        Logger::AddLog("Created local light source");
                    } else if (!showLightSource && light) {
                        if (isLightSelected) {
                            isLightSelected = false;
                        }
                        delete light;
                        light = nullptr;
                        data.lightMesh = nullptr;
                        Logger::AddLog("Destroyed local light source");
                    }
                }
                if (ImGui::Checkbox("Show Plane", &showPlane)) {
                    if (showPlane && !plane) {
                        plane = new Model("../Resource/obj/14082_WWII_Plane_Japan_Kawasaki_Ki-61_v1_L2.obj", false);
                        for (auto& mesh : plane->meshes) {
                            std::vector<Texture> defaultTextures;
                            defaultTextures.emplace_back("../Resource/default/texture/DefaultTex.png", "diffuse", 0);
                            defaultTextures.emplace_back("../Resource/default/texture/DefaultTex.png", "specular", 1);
                            mesh.textures = defaultTextures;
                        }
                        data.plane = plane;
                        Logger::AddLog("Created plane object with default texture");
                    } else if (!showPlane && plane) {
                        if (selectedMesh != -1) {
                            selectedMesh = -1;
                        }
                        delete plane;
                        plane = nullptr;
                        data.plane = nullptr;
                        Logger::AddLog("Destroyed plane object");
                    }
                }


            }

            if (ImGui::CollapsingHeader("Graphics")) {
                if (ImGui::SliderFloat("Render Distance", data.farPlane, 10.0f, 1000.0f, "%.1f")) {
                    camera.farPlane = *data.farPlane;
                }
                if (ImGui::SliderFloat("Field of View", &camera.FOV, 30.0f, 120.0f, "%.1f")) {
                    // FOV is updated in the camera
                }
                if (ImGui::SliderFloat("Near Plane", &camera.nearPlane, 0.01f, 1.0f, "%.3f")) {
                    // Near plane is updated in the camera
                }
                if (ImGui::Combo("MSAA", &data.currentMsaaIndex, msaaOptions, IM_ARRAYSIZE(msaaOptions))) {
                    createMsaaFramebuffer(msaaSamplesValues[data.currentMsaaIndex]);
                }
            }

            if (showPerformance && ImGui::CollapsingHeader("Performance")) {
                ImGui::Text("FPS: %.1f", fps);
                ImGui::Text("Frame Time: %.2f ms", avgFrameTime);
                ImGui::Text("Delta Time: %.4f s", deltaTime);
                ImGui::Separator();
                ImGui::Text("Scene Objects: %zu", cubes.size());
                ImGui::Text("MSAA Samples: %d", data.msaaSamples);
                ImGui::Text("Render Distance: %.1f", camera.farPlane);
                ImGui::Text("Near Plane: %.3f", camera.nearPlane);
                ImGui::Text("Field of View: %.1f°", camera.FOV);
            }

            if (ImGui::CollapsingHeader("Sun Settings")) {
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Primary Light Source");
                ImGui::SameLine();
                ImGui::TextDisabled("(Objects are lit by the sun)");
                ImGui::Separator();
                
                static bool showSun = true;
                if (ImGui::Checkbox("Show Sun", &showSun)) {
                    if (showSun && !sun) {
                        Mesh sunSphere = ObjectFactory::createSphere(32, 16);
                        sun = new Mesh(sunSphere);
                        data.sunMesh = sun;
                        Logger::AddLog("Created sun");
                    } else if (!showSun && sun) {
                        if (isSunSelected) {
                            isSunSelected = false;
                        }
                        delete sun;
                        sun = nullptr;
                        data.sunMesh = nullptr;
                        Logger::AddLog("Destroyed sun");
                    }
                }
                
                if (sun) {
                    ImGui::Separator();
                    ImGui::Text("Sun Properties");
                    
                    // Sun color picker
                    float sunColorArray[4] = { sunColor.x, sunColor.y, sunColor.z, sunColor.w };
                    if (ImGui::ColorEdit4("Sun Color", sunColorArray)) {
                        sunColor = glm::vec4(sunColorArray[0], sunColorArray[1], sunColorArray[2], sunColorArray[3]);
                        updateSunUniforms(shaderProgram, sunProgram, sunColor, sunPos, sunIntensity);
                    }
                    
                    // Sun intensity slider
                    if (ImGui::SliderFloat("Sun Intensity", &sunIntensity, 0.0f, 10.0f)) {
                        updateSunUniforms(shaderProgram, sunProgram, sunColor, sunPos, sunIntensity);
                    }
                    
                    ImGui::Separator();
                    ImGui::Text("Texture Settings");
                    
                    // Global texture tiling factor
                    if (ImGui::SliderFloat("Global Texture Tiling", &globalTilingFactor, 0.1f, 10.0f)) {
                    }
                    
                    // Sun position controls
                    ImGui::Separator();
                    ImGui::Text("Sun Position");
                    float sunPosArray[3] = { sunPos.x, sunPos.y, sunPos.z };
                    if (ImGui::DragFloat3("Position", sunPosArray, 0.1f)) {
                        sunPos = glm::vec3(sunPosArray[0], sunPosArray[1], sunPosArray[2]);
                        updateSunUniforms(shaderProgram, sunProgram, sunColor, sunPos, sunIntensity);
                    }
                    
                    // Preset sun positions
                    if (ImGui::Button("Dawn")) {
                        sunPos = glm::vec3(-10.0f, 5.0f, 0.0f);
                        sunColor = glm::vec4(1.0f, 0.6f, 0.3f, 1.0f); // Orange-red
                        sunIntensity = 1.5f;
                        updateSunUniforms(shaderProgram, sunProgram, sunColor, sunPos, sunIntensity);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Noon")) {
                        sunPos = glm::vec3(0.0f, 20.0f, 0.0f);
                        sunColor = glm::vec4(1.0f, 1.0f, 0.9f, 1.0f); // Bright white
                        sunIntensity = 2.5f;
                        updateSunUniforms(shaderProgram, sunProgram, sunColor, sunPos, sunIntensity);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Dusk")) {
                        sunPos = glm::vec3(10.0f, 5.0f, 0.0f);
                        sunColor = glm::vec4(1.0f, 0.4f, 0.2f, 1.0f); // Deep orange
                        sunIntensity = 1.2f;
                        updateSunUniforms(shaderProgram, sunProgram, sunColor, sunPos, sunIntensity);
                    }
                }
                
                ImGui::Separator();
                ImGui::Text("Bloom Settings");
                ImGui::SliderFloat("Sun Bloom", &sunBloom, 0.0f, 0.1f);
                ImGui::SliderFloat("Moon Bloom", &moonBloom, 0.0f, 0.1f);
            }

            if (ImGui::CollapsingHeader("Create")) {
                if (ImGui::Button("Add Cube")) {
                    Mesh newCube = ObjectFactory::createCube();
                    cubes.push_back(SceneObject(newCube));
                    Logger::AddLog("Added a new cube");
                }
                if (ImGui::Button("Add Plane")) {
					Mesh newPlane = ObjectFactory::createPlane();
					cubes.push_back(SceneObject(newPlane));
					Logger::AddLog("Added a new plane");
				}
				if (ImGui::Button("Add Sphere")) {
					Mesh newSphere = ObjectFactory::createSphere(32, 16);
					cubes.push_back(SceneObject(newSphere));
					Logger::AddLog("Added a new sphere");
				}
                
                ImGui::Separator();
                if (ImGui::Button("Delete Selected", ImVec2(ImGui::GetWindowWidth() - 20, 0))) {
                    if (selectedCube != -1) {
                        cubes.erase(cubes.begin() + selectedCube);
                        selectedCube = -1;
                        Logger::AddLog("Deleted selected object");
                    } else if (selectedMesh != -1) {
                        selectedMesh = -1;
                        Logger::AddLog("Cleared plane selection");
                    } else if (isLightSelected) {
                        isLightSelected = false;
                        Logger::AddLog("Cleared light selection");
                    } else if (isSunSelected) {
                        isSunSelected = false;
                        Logger::AddLog("Cleared sun selection");
                    }
                }
                
                if (ImGui::Button("Destroy All Objects", ImVec2(ImGui::GetWindowWidth() - 20, 0))) {
                    // Clear all cubes (Mesh destructor will handle cleanup)
                    cubes.clear();
                    
                    // Clear selection states
                    selectedCube = -1;
                    selectedMesh = -1;
                    isLightSelected = false;
                    isSunSelected = false;
                    
                    Logger::AddLog("Destroyed all objects");
                }
                 
                


                if (ImGui::Button("Clear Selection", ImVec2(ImGui::GetWindowWidth() - 20, 0))) {
                    selectedCube = -1;
                    selectedMesh = -1;
                    isLightSelected = false;
                    isSunSelected = false;
                    Logger::AddLog("Cleared all selections");
                }
            }

            if (ImGui::CollapsingHeader("Editor Mode")) {
                if (ImGui::RadioButton("Edit", Editor::isEditMode)) {
                    Editor::isEditMode = true;
                }
                ImGui::SameLine();
                if (ImGui::RadioButton("Play", !Editor::isEditMode)) {
                    Editor::isEditMode = false;
                }
            }

            if (Editor::isEditMode) {
                if (ImGui::CollapsingHeader("Transform")) {
                    if (ImGui::RadioButton("Translate", Editor::selectionMode == Editor::TRANSLATE)) {
                        Editor::selectionMode = Editor::TRANSLATE;
                        gizmo.SetMode(Gizmo::TRANSLATE);
                    }
                    ImGui::SameLine();
                    if (ImGui::RadioButton("Rotate", Editor::selectionMode == Editor::ROTATE)) {
                        Editor::selectionMode = Editor::ROTATE;
                        gizmo.SetMode(Gizmo::ROTATE);
                    }
                    ImGui::SameLine();
                    if (ImGui::RadioButton("Scale", Editor::selectionMode == Editor::SCALE)) {
                        Editor::selectionMode = Editor::SCALE;
                        gizmo.SetMode(Gizmo::SCALE);
                    }
                    
                    // Transformation Value Sliders
                    ImGui::Separator();
                    ImGui::Text("Manual Transformation Values:");
                    
                    // Position sliders
                    if (selectedCube != -1) {
                        ImGui::Text("Position:");
                        static float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
                        if (ImGui::SliderFloat("X##Pos", &posX, -10.0f, 10.0f, "%.2f")) {
                            cubes[selectedCube].position.x = posX;
                        }
                        if (ImGui::SliderFloat("Y##Pos", &posY, -10.0f, 10.0f, "%.2f")) {
                            cubes[selectedCube].position.y = posY;
                        }
                        if (ImGui::SliderFloat("Z##Pos", &posZ, -10.0f, 10.0f, "%.2f")) {
                            cubes[selectedCube].position.z = posZ;
                        }
                        
                        // Update slider values when object moves via gizmo
                        posX = cubes[selectedCube].position.x;
                        posY = cubes[selectedCube].position.y;
                        posZ = cubes[selectedCube].position.z;
                        
                        // Rotation sliders (in degrees)
                        ImGui::Text("Rotation (degrees):");
                        static float rotX = 0.0f, rotY = 0.0f, rotZ = 0.0f;
                        glm::vec3 eulerAngles = glm::degrees(glm::eulerAngles(cubes[selectedCube].rotation));
                        
                        if (ImGui::SliderFloat("X##Rot", &rotX, -180.0f, 180.0f, "%.1f")) {
                            cubes[selectedCube].rotation = glm::quat(glm::radians(glm::vec3(rotX, eulerAngles.y, eulerAngles.z)));
                        }
                        if (ImGui::SliderFloat("Y##Rot", &rotY, -180.0f, 180.0f, "%.1f")) {
                            cubes[selectedCube].rotation = glm::quat(glm::radians(glm::vec3(eulerAngles.x, rotY, eulerAngles.z)));
                        }
                        if (ImGui::SliderFloat("Z##Rot", &rotZ, -180.0f, 180.0f, "%.1f")) {
                            cubes[selectedCube].rotation = glm::quat(glm::vec3(eulerAngles.x, eulerAngles.y, glm::radians(rotZ)));
                        }
                        
                        // Update slider values when object rotates via gizmo
                        rotX = eulerAngles.x;
                        rotY = eulerAngles.y;
                        rotZ = eulerAngles.z;
                        
                        // Scale sliders
                        ImGui::Text("Scale:");
                        static float scaleX = 1.0f, scaleY = 1.0f, scaleZ = 1.0f;
                        if (ImGui::SliderFloat("X##Scale", &scaleX, 0.1f, 10.0f, "%.2f")) {
                            cubes[selectedCube].scale.x = scaleX;
                        }
                        if (ImGui::SliderFloat("Y##Scale", &scaleY, 0.1f, 10.0f, "%.2f")) {
                            cubes[selectedCube].scale.y = scaleY;
                        }
                        if (ImGui::SliderFloat("Z##Scale", &scaleZ, 0.1f, 10.0f, "%.2f")) {
                            cubes[selectedCube].scale.z = scaleZ;
                        }
                        
                        // Update slider values when object scales via gizmo
                        scaleX = cubes[selectedCube].scale.x;
                        scaleY = cubes[selectedCube].scale.y;
                        scaleZ = cubes[selectedCube].scale.z;
                        
                        // Uniform scale slider
                        ImGui::Text("Uniform Scale:");
                        static float uniformScale = 1.0f;
                        if (ImGui::SliderFloat("Scale All", &uniformScale, 0.1f, 10.0f, "%.2f")) {
                            cubes[selectedCube].scale = glm::vec3(uniformScale);
                        }
                        
                        // Update uniform scale when individual scales change
                        if (scaleX == scaleY && scaleY == scaleZ) {
                            uniformScale = scaleX;
                        }
                    } else if (selectedMesh != -1) {
                        ImGui::Text("Plane selected - use gizmo for transformation");
                    } else if (isLightSelected) {
                        ImGui::Text("Light selected - use gizmo for transformation");
                    } else if (isSunSelected) {
                        ImGui::Text("Sun selected - use gizmo for transformation");
                    } else {
                        ImGui::Text("Select an object to edit its transformation values");
                    }
                }
                

                
                // Selection info and Object Management
                if (ImGui::CollapsingHeader("Selection & Objects")) {
                    if (selectedCube != -1) {
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Selected: Cube %d", selectedCube);
                        ImGui::Text("Position: %.2f, %.2f, %.2f", cubes[selectedCube].position.x, cubes[selectedCube].position.y, cubes[selectedCube].position.z);
                        ImGui::Text("Scale: %.2f, %.2f, %.2f", cubes[selectedCube].scale.x, cubes[selectedCube].scale.y, cubes[selectedCube].scale.z);
                        
                        // Duplicate cube button
                        ImGui::Separator();
                        if (ImGui::Button("Duplicate Cube", ImVec2(ImGui::GetWindowWidth() - 20, 0))) {
                            if (selectedCube >= 0 && selectedCube < cubes.size()) {
                                SceneObject newCube = cubes[selectedCube];
                                newCube.position += glm::vec3(1.0f, 0.0f, 0.0f); // Offset slightly
                                cubes.push_back(newCube);
                                Logger::AddLog("Duplicated cube");
                            }
                        }
                    } else if (selectedMesh != -1) {
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Selected: Plane Mesh %d", selectedMesh);
                        ImGui::Text("Note: Plane transformations are limited");
                    } else if (isLightSelected) {
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Selected: Light Source");
                        ImGui::Text("Position: %.2f, %.2f, %.2f", lightPos.x, lightPos.y, lightPos.z);
                    } else if (isSunSelected) {
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Selected: Sun");
                        ImGui::Text("Position: %.2f, %.2f, %.2f", sunPos.x, sunPos.y, sunPos.z);
                    } else {
                        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No object selected");
                        ImGui::Text("Click on any object to select it");
                    }
                    
                    // Object count info
                    ImGui::Separator();
                    ImGui::Text("Scene Objects:");
                    ImGui::BulletText("Cubes: %zu", cubes.size());
                    ImGui::BulletText("Plane: %s", showPlane ? "Visible" : "Hidden");
                    ImGui::BulletText("Light: %s", showLightSource ? "Visible" : "Hidden");
                    ImGui::BulletText("Sun: %s", sun ? "Visible" : "Hidden");
                }
                

                
                // Camera Settings
                if (ImGui::CollapsingHeader("Camera")) {
                    ImGui::Checkbox("Free Movement", &Editor::isFreeMovement);
                    ImGui::SliderFloat("Mouse Sensitivity", &camera.sensitivity, 0.0f, 200.0f);
                    ImGui::SliderFloat("Camera Speed", &camera.speed, 0.1f, 10.0f, "%.2f");
                    
                    // Camera info
                    ImGui::Separator();
                    ImGui::Text("Camera Info:");
                    ImGui::Text("Position: %.2f, %.2f, %.2f", camera.Position.x, camera.Position.y, camera.Position.z);
                    ImGui::Text("Speed: %.2f", camera.speed);
                    ImGui::Text("Sensitivity: %.1f", camera.sensitivity);
                }
            }

            Logger::Draw("Logger");

            ImGui::End();

    		// Bind the custom framebuffer
            if (data.msaaSamples > 0) {
                glBindFramebuffer(GL_FRAMEBUFFER, data.msaaFBO);
            } else {
    		    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
            }
    	glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
    	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    	   if (showGradientSky)
    	   {
    	       glDisable(GL_DEPTH_TEST);
    	       gradientSkyProgram.use();
    	       glUniformMatrix4fv(glGetUniformLocation(gradientSkyProgram.ID, "view"), 1, GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));
    	       glUniformMatrix4fv(glGetUniformLocation(gradientSkyProgram.ID, "projection"), 1, GL_FALSE, glm::value_ptr(camera.GetProjectionMatrix()));
    	       
    	       float time = glfwGetTime();
    	       float dayNightCycle = sin(time * 0.1) * 0.5 + 0.5;

    	       glm::vec3 dayTopColor = glm::vec3(0.5f, 0.7f, 1.0f);
    	       glm::vec3 nightTopColor = glm::vec3(0.0f, 0.0f, 0.1f);
    	       glm::vec3 dayBottomColor = glm::vec3(1.0f, 1.0f, 1.0f);
    	       glm::vec3 nightBottomColor = glm::vec3(0.1f, 0.1f, 0.2f);

    	       glm::vec3 topColor = mix(nightTopColor, dayTopColor, dayNightCycle);
    	       glm::vec3 bottomColor = mix(nightBottomColor, dayBottomColor, dayNightCycle);

    	       glUniform3f(glGetUniformLocation(gradientSkyProgram.ID, "topColor"), topColor.x, topColor.y, topColor.z);
    	       glUniform3f(glGetUniformLocation(gradientSkyProgram.ID, "bottomColor"), bottomColor.x, bottomColor.y, bottomColor.z);

    	       glm::vec3 DynamicSunPos = glm::vec3(cos(time * 0.1) * 10.0f, sin(time * 0.1) * 10.0f, 0.0f);
    	       glm::vec3 DynamicMoonPos = glm::vec3(cos(time * 0.1 + glm::pi<float>()) * 10.0f, sin(time * 0.1 + glm::pi<float>()) * 10.0f, 0.0f);

    	       updateGradientSkyUniforms(gradientSkyProgram, DynamicSunPos, DynamicMoonPos, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(0.8f, 0.8f, 0.9f, 1.0f), sunBloom, moonBloom);

    	       glBindVertexArray(skyVAO);
    	       glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    	       glEnable(GL_DEPTH_TEST);
    	   }

    	glEnable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
   
   
    	// Handles camera inputs
    	if (Editor::isEditMode) {
    		camera.Inputs(window);
    	}
    //	floor.Draw(shaderProgram, camera);
        glStencilFunc(GL_ALWAYS, 0, 0xFF);
        glStencilMask(0xFF);

        if (showLightSource && light)
        {
            glm::vec3 lightScale = glm::vec3(1.0f); // Light scale
            
            if (Editor::isEditMode && isLightSelected) {
                selectionProgram.use();
                glUniform4f(glGetUniformLocation(selectionProgram.ID, "sunColor"), sunColor.x, sunColor.y, sunColor.z, sunColor.w);
                glUniform3f(glGetUniformLocation(selectionProgram.ID, "sunPos"), sunPos.x, sunPos.y, sunPos.z);
                glUniform1f(glGetUniformLocation(selectionProgram.ID, "sunIntensity"), sunIntensity);
                glUniform3f(glGetUniformLocation(selectionProgram.ID, "camPos"), camera.Position.x, camera.Position.y, camera.Position.z);
                light->Draw(selectionProgram, camera, lightPos, glm::quat(1.0f, 0.0f, 0.0f, 0.0f), lightScale);
            } else {
                light->Draw(lightShader, camera, lightPos, glm::quat(1.0f, 0.0f, 0.0f, 0.0f), lightScale);
            }
        }

        // Draw the sun
        if (sun)
        {
            glm::vec3 sunScale = glm::vec3(2.0f);
            
            if (Editor::isEditMode && isSunSelected) {
                // Draw selected sun with selection shader (bright red)
                selectionProgram.use();
                // Set sun uniforms for lighting
                glUniform4f(glGetUniformLocation(selectionProgram.ID, "sunColor"), sunColor.x, sunColor.y, sunColor.z, sunColor.w);
                glUniform3f(glGetUniformLocation(selectionProgram.ID, "sunPos"), sunPos.x, sunPos.y, sunPos.z);
                glUniform1f(glGetUniformLocation(selectionProgram.ID, "sunIntensity"), sunIntensity);
                glUniform3f(glGetUniformLocation(selectionProgram.ID, "camPos"), camera.Position.x, camera.Position.y, camera.Position.z);
                sun->Draw(selectionProgram, camera, sunPos, glm::quat(1.0f, 0.0f, 0.0f, 0.0f), sunScale);
            } else {
                // Draw normal sun
                sunProgram.use();
                glUniform4f(glGetUniformLocation(sunProgram.ID, "sunColor"), sunColor.x, sunColor.y, sunColor.z, sunColor.w);
                glUniform1f(glGetUniformLocation(sunProgram.ID, "sunIntensity"), sunIntensity);
                glUniform3f(glGetUniformLocation(sunProgram.ID, "camPos"), camera.Position.x, camera.Position.y, camera.Position.z);
                sun->Draw(sunProgram, camera, sunPos, glm::quat(1.0f, 0.0f, 0.0f, 0.0f), sunScale);
            }
        }

        // Draw the plane
        if (showPlane && plane)
        {
            glm::vec3 planeScale = glm::vec3(1.0f);
            for (int i = 0; i < plane->meshes.size(); ++i)
            {
                if (Editor::isEditMode && i == selectedMesh) {
                    // Draw selected plane mesh with selection shader (bright red)
                    selectionProgram.use();
                    // Set sun uniforms for lighting
                    glUniform4f(glGetUniformLocation(selectionProgram.ID, "sunColor"), sunColor.x, sunColor.y, sunColor.z, sunColor.w);
                    glUniform3f(glGetUniformLocation(selectionProgram.ID, "sunPos"), sunPos.x, sunPos.y, sunPos.z);
                    glUniform1f(glGetUniformLocation(selectionProgram.ID, "sunIntensity"), sunIntensity);
                    glUniform3f(glGetUniformLocation(selectionProgram.ID, "camPos"), camera.Position.x, camera.Position.y, camera.Position.z);
                    plane->meshes[i].Draw(selectionProgram, camera, glm::vec3(0.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), planeScale);
                } else {
                    // Draw normal plane mesh
                    plane->meshes[i].Draw(shaderProgram, camera, glm::vec3(0.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), planeScale);
                }
            }
        }

        // Draw the cubes
        for (int i = 0; i < cubes.size(); ++i)
        {
            // Apply global tiling factor to scale for texture tiling
            glm::vec3 tiledScale = cubes[i].scale * globalTilingFactor;
            
            if (Editor::isEditMode && i == selectedCube) {
                // Draw selected cube with selection shader (bright red)
                selectionProgram.use();
                // Set sun uniforms for lighting
                glUniform4f(glGetUniformLocation(selectionProgram.ID, "sunColor"), sunColor.x, sunColor.y, sunColor.z, sunColor.w);
                glUniform3f(glGetUniformLocation(selectionProgram.ID, "sunPos"), sunPos.x, sunPos.y, sunPos.z);
                glUniform1f(glGetUniformLocation(selectionProgram.ID, "sunIntensity"), sunIntensity);
                glUniform3f(glGetUniformLocation(selectionProgram.ID, "camPos"), camera.Position.x, camera.Position.y, camera.Position.z);
                cubes[i].mesh.Draw(selectionProgram, camera, cubes[i].position, cubes[i].rotation, tiledScale);
            } else {
                // Draw normal cube
                cubes[i].mesh.Draw(shaderProgram, camera, cubes[i].position, cubes[i].rotation, tiledScale);
            }
        }

        if (Editor::isEditMode)
        {
            if (selectedMesh != -1 && plane) {
                gizmo.Draw(gizmoProgram, camera, plane->meshes[selectedMesh].vertices[0].position);

            }
            if (selectedCube != -1) {
                gizmo.Draw(gizmoProgram, camera, cubes[selectedCube].position);
                
                // Handle gizmo interaction for cubes
                glm::vec3 newPosition = cubes[selectedCube].position;
                glm::vec3 newRotation = glm::degrees(glm::eulerAngles(cubes[selectedCube].rotation));
                glm::vec3 newScale = cubes[selectedCube].scale;
                
                gizmo.HandleMouse(window, camera, cubes[selectedCube].position, 
                                newPosition, newRotation, newScale);
                
                if (gizmo.GetMode() == Gizmo::TRANSLATE && newPosition != cubes[selectedCube].position) {
                    cubes[selectedCube].position = newPosition;
                }
                if (gizmo.GetMode() == Gizmo::ROTATE && (newRotation.x != 0.0f || newRotation.y != 0.0f || newRotation.z != 0.0f)) {
                    glm::quat rotationDelta = glm::quat(glm::radians(newRotation));
                    cubes[selectedCube].rotation = rotationDelta * cubes[selectedCube].rotation;
                }
                if (gizmo.GetMode() == Gizmo::SCALE && (newScale.x != 0.0f || newScale.y != 0.0f || newScale.z != 0.0f)) {

                    cubes[selectedCube].scale += newScale;

                    cubes[selectedCube].scale.x = glm::clamp(cubes[selectedCube].scale.x, 0.1f, 10.0f);
                    cubes[selectedCube].scale.y = glm::clamp(cubes[selectedCube].scale.y, 0.1f, 10.0f);
                    cubes[selectedCube].scale.z = glm::clamp(cubes[selectedCube].scale.z, 0.1f, 10.0f);
                }
            }
            if (isLightSelected && light) {
                gizmo.Draw(gizmoProgram, camera, lightPos);

                glm::vec3 newPosition = lightPos;
                glm::vec3 newRotation(0.0f);
                glm::vec3 newScale(1.0f);
                gizmo.HandleMouse(window, camera, lightPos, newPosition, newRotation, newScale);

                if (gizmo.GetMode() == Gizmo::TRANSLATE && newPosition != lightPos) {
                    lightPos = newPosition;
                }
            }
            if (isSunSelected && sun) {
                gizmo.Draw(gizmoProgram, camera, sunPos);
                glm::vec3 newPosition = sunPos;
                glm::vec3 newRotation(0.0f);
                glm::vec3 newScale(1.0f);
                gizmo.HandleMouse(window, camera, sunPos, newPosition, newRotation, newScale);
                
                if (gizmo.GetMode() == Gizmo::TRANSLATE && newPosition != sunPos) {
                    sunPos = newPosition;
                    updateSunUniforms(shaderProgram, sunProgram, sunColor, sunPos, sunIntensity);
                }
            }
        }
        // Sky rendering
        if (showCubemap)
        {
            glDepthFunc(GL_LEQUAL);
            skyboxShader.use();
            glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
            glm::mat4 projection = camera.GetProjectionMatrix();
            glUniformMatrix4fv(glGetUniformLocation(skyboxShader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(skyboxShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
            glBindVertexArray(skyboxVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
            glDepthFunc(GL_LESS);
        }
        else if (showGradientSky)
        {
            if (showClouds) {
                glm::mat4 cloudModel = glm::mat4(1.0f);
                cloudModel = glm::translate(cloudModel, glm::vec3(0.0f, 20.0f, 0.0f));
                cloudModel = glm::scale(cloudModel, glm::vec3(100.0f, 100.0f, 100.0f));
                cloud.Draw(cloudProgram, camera, cloudModel);
            }
        }
   
        if (data.msaaSamples > 0) {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, data.msaaFBO);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
            glBlitFramebuffer(0, 0, camera.width, camera.height, 0, 0, camera.width, camera.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        }
   
    	glBindFramebuffer(GL_FRAMEBUFFER, 0);
    	framebufferProgram.use();
    	glUniform1i(glGetUniformLocation(framebufferProgram.ID, "screenTexture"), 0);
    	glDisable(GL_DEPTH_TEST);
    	// Draw the framebuffer rectangle
    	glBindVertexArray(rectVAO);
    	glActiveTexture(GL_TEXTURE0);
    	glBindTexture(GL_TEXTURE_2D, framebufferTexture);
    	glDrawArrays(GL_TRIANGLES, 0, 6);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    	glfwSwapBuffers(window);
    	glfwPollEvents();
    }


    shaderProgram.Delete();
    lightShader.Delete();
    framebufferProgram.Delete();
    skyboxShader.Delete();
    selectionProgram.Delete();
    sunProgram.Delete();
    horizonProgram.Delete();
    gradientSkyProgram.Delete();
    if (plane) delete plane;
    if (light) delete light;
    if (sun) delete sun;
    glDeleteFramebuffers(1, &FBO);
    if (data.msaaSamples > 0) {
        glDeleteFramebuffers(1, &data.msaaFBO);
        glDeleteTextures(1, &data.msaaColorBuffer);
        glDeleteRenderbuffers(1, &data.msaaRBO);
    }
    glDeleteVertexArrays(1, &rectVAO);
    glDeleteBuffers(1, &rectVBO);
    glDeleteRenderbuffers(1, &RBO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteBuffers(1, &skyboxEBO);
    //lightShader.Delete();
   
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
   
   void framebuffer_size_callback(GLFWwindow* window, int width, int height)
   {
   	glViewport(0, 0, width, height);
   	WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);
   	if (data && data->camera)
   	{
   		data->camera->width = width;
   		data->camera->height = height;
   	}
   	// Recreate framebuffer texture and RBO with new size
   	glBindTexture(GL_TEXTURE_2D, data->framebufferTexture);
   	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
   	glBindRenderbuffer(GL_RENDERBUFFER, data->RBO);
   	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    if (data->msaaSamples > 0) {
        glBindRenderbuffer(GL_RENDERBUFFER, data->msaaRBO);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, data->msaaSamples, GL_DEPTH24_STENCIL8, width, height);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, data->msaaColorBuffer);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, data->msaaSamples, GL_RGB, width, height, GL_TRUE);
    }
   }

   void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
   {
   	   if (key == GLFW_KEY_F3 && action == GLFW_PRESS)
   	   {
   	       Editor::ToggleMode();
   	   }
   	      if (key == GLFW_KEY_F4 && action == GLFW_PRESS)
   	      {
   	          Editor::ToggleFreeMovement();
   	      }
   	      if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
   	      {
   	          Editor::isFreeMovement = false;
   	      }
   	      if (key == GLFW_KEY_C && action == GLFW_PRESS && (mods & GLFW_MOD_CONTROL))
   	      {
   	          // Ctrl+C to clear selection
   	          WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);
   	          if (data) {
   	              *data->selectedCube = -1;
   	              *data->selectedMesh = -1;
   	              *data->isLightSelected = false;
   	              *data->isSunSelected = false;
   	              Logger::AddLog("Cleared all selections (Ctrl+C)");
   	          }
   	      }
  if (key == GLFW_KEY_DELETE && action == GLFW_PRESS)
  {
   WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);
   if (data) {
       if (*data->selectedCube != -1) {
           data->cubes->erase(data->cubes->begin() + *data->selectedCube);
           *data->selectedCube = -1;
           Logger::AddLog("Deleted selected object (Delete key)");
       } else if (*data->selectedMesh != -1) {
           *data->selectedMesh = -1;
           Logger::AddLog("Cleared plane selection (Delete key)");
       } else if (*data->isLightSelected) {
           *data->isLightSelected = false;
           Logger::AddLog("Cleared light selection (Delete key)");
       } else if (*data->isSunSelected) {
           *data->isSunSelected = false;
           Logger::AddLog("Cleared sun selection (Delete key)");
       }
   }
  }
   }

   void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        if (ImGui::GetIO().WantCaptureMouse) {
            return;
        }
        
        if (Editor::isEditMode)
        {
            WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);
            glm::vec3 ray = data->camera->GetRay(window);
            
            float closest_intersection = std::numeric_limits<float>::max();
            bool hitSomething = false;
            

            if (data->plane) {
                for (int i = 0; i < data->plane->meshes.size(); ++i) {
                    float intersection_distance;
                    glm::mat4 modelMatrix = glm::mat4(1.0f);
                    if (data->plane->meshes[i].Intersect(data->camera->Position, ray, modelMatrix, intersection_distance)) {
                        if (intersection_distance < closest_intersection) {
                            closest_intersection = intersection_distance;
                            *data->selectedMesh = i;
                            *data->selectedCube = -1;
                            *data->isLightSelected = false;
                            *data->isSunSelected = false;
                            hitSomething = true;
                            Logger::AddLog("Selected plane mesh %d", i);
                        }
                    }
                }
            }
            for (int i = 0; i < data->cubes->size(); ++i) {
                float intersection_distance;
                glm::mat4 modelMatrix = glm::mat4(1.0f);
                modelMatrix = glm::translate(modelMatrix, data->cubes->at(i).position);
                modelMatrix *= glm::mat4_cast(data->cubes->at(i).rotation);
                modelMatrix = glm::scale(modelMatrix, data->cubes->at(i).scale);
                if (data->cubes->at(i).mesh.Intersect(data->camera->Position, ray, modelMatrix, intersection_distance)) {
                    if (intersection_distance < closest_intersection) {
                        closest_intersection = intersection_distance;
                        *data->selectedCube = i;
                        *data->selectedMesh = -1;
                        *data->isLightSelected = false;
                        *data->isSunSelected = false;
                        hitSomething = true;
                        Logger::AddLog("Selected cube %d", i);
                    }
                }
            }
            if (data->lightMesh) {
                float intersection_distance;
                glm::mat4 modelMatrix = glm::mat4(1.0f);
                modelMatrix = glm::translate(modelMatrix, *data->lightPos);
                if (data->lightMesh->Intersect(data->camera->Position, ray, modelMatrix, intersection_distance)) {
                    if (intersection_distance < closest_intersection) {
                        closest_intersection = intersection_distance;
                        *data->selectedCube = -1;
                        *data->selectedMesh = -1;
                        *data->isLightSelected = true;
                        *data->isSunSelected = false;
                        hitSomething = true;
                        Logger::AddLog("Selected light source");
                    }
                }
            }
            
            // Check sun selection
            if (data->sunMesh) {
                float intersection_distance;
                glm::mat4 modelMatrix = glm::mat4(1.0f);
                modelMatrix = glm::translate(modelMatrix, *data->sunPos);
                modelMatrix = glm::scale(modelMatrix, glm::vec3(2.0f)); // Scale for sun size
                if (data->sunMesh->Intersect(data->camera->Position, ray, modelMatrix, intersection_distance)) {
                    if (intersection_distance < closest_intersection) {
                        closest_intersection = intersection_distance;
                        *data->selectedCube = -1;
                        *data->selectedMesh = -1;
                        *data->isLightSelected = false;
                        *data->isSunSelected = true;
                        hitSomething = true;
                        Logger::AddLog("Selected sun");
                    }
                }
            }
            
            // Only clear selection if we didn't hit anything (including gizmo)
            if (!hitSomething) {
                *data->selectedMesh = -1;
                *data->selectedCube = -1;
                *data->isLightSelected = false;
                *data->isSunSelected = false;
            }
        }
    }
}
// test2