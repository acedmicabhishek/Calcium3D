#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>

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
    int msaaSamples;
    unsigned int msaaFBO;
    unsigned int msaaColorBuffer;
    unsigned int msaaRBO;
    int currentMsaaIndex;
};
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

int main() {
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
    Shader celShadingProgram("../shaders/cel_shading.vert", "../shaders/cel_shading.frag");
    Shader gizmoProgram("../shaders/gizmo.vert", "../shaders/gizmo.frag");
    std::vector <Vertex> verts(vertices, vertices + sizeof(vertices) / sizeof(Vertex));
    std::vector <GLuint> ind(indices, indices + sizeof(indices) / sizeof(GLuint));
   
   
    // Shader for light cube
    Shader lightShader("../shaders/light.vert", "../shaders/light.frag");
    Shader skyboxShader("../shaders/skybox.vert", "../shaders/skybox.frag");
    std::vector <Vertex> lightVerts(lightVertices, lightVertices + sizeof(lightVertices) / sizeof(Vertex));
    std::vector <GLuint> lightInd(lightIndices, lightIndices + sizeof(lightIndices) / sizeof(GLuint));
   
   
   
    glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    glm::vec3 lightPos = glm::vec3(0.5f, 0.5f, 0.5f);
    glm::mat4 lightModel = glm::mat4(1.0f);
    lightModel = glm::translate(lightModel, lightPos);

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


    // Textures
 Texture textures[]
 {
  Texture("../Resource/texture/planks.png", "diffuse", 0),
  Texture("../Resource/texture/planksSpec.png", "specular", 1)
 };
 std::vector <Texture> tex(textures, textures + sizeof(textures) / sizeof(Texture));
 Mesh floor(verts, ind, tex);
 Mesh light(lightVerts, lightInd, tex);
 Model plane("../Resource/obj/14082_WWII_Plane_Japan_Kawasaki_Ki-61_v1_L2.obj", false);
 Gizmo gizmo;
 int selectedMesh = -1;
 int selectedCube = -1;
 std::vector<SceneObject> cubes;

    // Enables the Depth Buffer
    glEnable(GL_DEPTH_TEST);

    // Creates camera object
    Camera camera(800, 600, glm::vec3(0.0f, 0.0f, 2.0f));
    unsigned int framebufferTexture;
    unsigned int RBO;
    WindowData data;
    data.camera = &camera;
    data.plane = &plane;
    data.cubes = &cubes;
    data.selectedMesh = &selectedMesh;
    data.selectedCube = &selectedCube;
    data.msaaSamples = 0;
    data.msaaFBO = 0;
    data.msaaColorBuffer = 0;
    data.msaaColorBuffer = 0;
    data.msaaRBO = 0;
    data.currentMsaaIndex = 0;
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

    const char* msaaOptions[] = { "Off", "2x", "4x", "8x" };
    int msaaSamplesValues[] = { 0, 2, 4, 8, 16 };

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
        bool showLightSource = true;
        bool showPlane = true;

    	while (!glfwWindowShouldClose(window))
    	{
            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2(300, ImGui::GetIO().DisplaySize.y));
            ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

            if (ImGui::CollapsingHeader("Display Options")) {
                ImGui::Checkbox("Show Cubemap", &showCubemap);
                ImGui::Checkbox("Show Light Source", &showLightSource);
                ImGui::Checkbox("Show Plane", &showPlane);
                if (ImGui::Combo("MSAA", &data.currentMsaaIndex, msaaOptions, IM_ARRAYSIZE(msaaOptions))) {
                    createMsaaFramebuffer(msaaSamplesValues[data.currentMsaaIndex]);
                }
            }

            if (ImGui::CollapsingHeader("Create")) {
                if (ImGui::Button("Add Cube")) {
                    Mesh newCube = ObjectFactory::createCube();
                    cubes.push_back(SceneObject(newCube));
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
                    }
                    ImGui::SameLine();
                    if (ImGui::RadioButton("Rotate", Editor::selectionMode == Editor::ROTATE)) {
                        Editor::selectionMode = Editor::ROTATE;
                    }
                    ImGui::SameLine();
                    if (ImGui::RadioButton("Scale", Editor::selectionMode == Editor::SCALE)) {
                        Editor::selectionMode = Editor::SCALE;
                    }
                }
                ImGui::Checkbox("Free Movement", &Editor::isFreeMovement);
                ImGui::SliderFloat("Sensitivity", &camera.sensitivity, 0.0f, 200.0f);
            }

            ImGui::End();

    		// Bind the custom framebuffer
            if (data.msaaSamples > 0) {
                glBindFramebuffer(GL_FRAMEBUFFER, data.msaaFBO);
            } else {
    		    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
            }
    	// Specify the color of the background
    	glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
    	// Clean the back buffer and depth buffer
    	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    	// Enable depth testing since it's disabled when drawing the framebuffer rectangle
    	glEnable(GL_DEPTH_TEST);
   
   
    	// Handles camera inputs
    	if (Editor::isEditMode) {
    		camera.Inputs(window);
    	}
    	// Updates and exports the camera matrix to the Vertex Shader
    
    
    	// Tells OpenGL which Shader Program we want to use
    //	floor.Draw(shaderProgram, camera);
        if (showLightSource)
        {
		    light.Draw(lightShader, camera, lightPos);
        }

        if (showPlane)
        {
		    plane.Draw(shaderProgram, camera, glm::vec3(0.0f, 0.0f, 0.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
		          if (Editor::isEditMode) {
		              if (selectedMesh != -1) {
		                  plane.meshes[selectedMesh].Draw(celShadingProgram, camera);
		                  gizmo.Draw(gizmoProgram, camera, plane.meshes[selectedMesh].vertices[0].position);
		              }
                      if (selectedCube != -1) {
                          cubes[selectedCube].mesh.Draw(celShadingProgram, camera, cubes[selectedCube].position, cubes[selectedCube].rotation, cubes[selectedCube].scale);
                          gizmo.Draw(gizmoProgram, camera, cubes[selectedCube].position);
                          gizmo.HandleMouse(window, camera, cubes[selectedCube].position, cubes[selectedCube].position);
                      }
		          }
		      }
            for (auto& cube : cubes) {
                cube.mesh.Draw(shaderProgram, camera, cube.position, cube.rotation, cube.scale);
            }
		   // Draw the skybox
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
   
        if (data.msaaSamples > 0) {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, data.msaaFBO);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
            glBlitFramebuffer(0, 0, camera.width, camera.height, 0, 0, camera.width, camera.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        }
   
    	// Bind the default framebuffer
    	glBindFramebuffer(GL_FRAMEBUFFER, 0);
    	// Use the framebuffer shader
    	framebufferProgram.use();
    	glUniform1i(glGetUniformLocation(framebufferProgram.ID, "screenTexture"), 0);
    	// Disable depth testing so the framebuffer rectangle is always drawn
    	glDisable(GL_DEPTH_TEST);
    	// Draw the framebuffer rectangle
    	glBindVertexArray(rectVAO);
    	glActiveTexture(GL_TEXTURE0);
    	glBindTexture(GL_TEXTURE_2D, framebufferTexture);
    	glDrawArrays(GL_TRIANGLES, 0, 6);
   
        // Rendering ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
   
    	// Swap the back buffer with the front buffer
    	glfwSwapBuffers(window);
    	glfwPollEvents();
    }



    // Delete all the objects we've created
    shaderProgram.Delete();
    lightShader.Delete();
    framebufferProgram.Delete();
    skyboxShader.Delete();
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
    // Delete window before ending the program
   
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
   }

   void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
   {
   	   if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
   	   {
   	       if (Editor::isEditMode)
   	       {
   	           WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);
   	           glm::vec3 ray = data->camera->GetRay(window);
   	           
   	           float closest_intersection = std::numeric_limits<float>::max();
   	           *data->selectedMesh = -1;
               *data->selectedCube = -1;

   	           for (int i = 0; i < data->plane->meshes.size(); ++i) {
   	               float intersection_distance;
   	               if (data->plane->meshes[i].Intersect(data->camera->Position, ray, intersection_distance)) {
   	                   if (intersection_distance < closest_intersection) {
   	                       closest_intersection = intersection_distance;
   	                       *data->selectedMesh = i;
                               *data->selectedCube = -1;
   	                   }
   	               }
   	           }
                for (int i = 0; i < data->cubes->size(); ++i) {
                    float intersection_distance;
                    if (data->cubes->at(i).mesh.Intersect(data->camera->Position, ray, intersection_distance)) {
                        if (intersection_distance < closest_intersection) {
                            closest_intersection = intersection_distance;
                            *data->selectedCube = i;
                            *data->selectedMesh = -1;
                        }
                    }
                }
   	       }
   	   }
   }
