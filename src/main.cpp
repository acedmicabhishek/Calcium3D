// src/main.cpp
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
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

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

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

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
 std::vector <Vertex> verts(vertices, vertices + sizeof(vertices) / sizeof(Vertex));
 std::vector <GLuint> ind(indices, indices + sizeof(indices) / sizeof(GLuint));


    // Shader for light cube
 Shader lightShader("../shaders/light.vert", "../shaders/light.frag");
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
  Texture("../Resource/texture/planks.png", "diffuse", 0, GL_RGBA, GL_UNSIGNED_BYTE),
  Texture("../Resource/texture/planksSpec.png", "specular", 1, GL_RED, GL_UNSIGNED_BYTE)
 };
 std::vector <Texture> tex(textures, textures + sizeof(textures) / sizeof(Texture));
 Mesh floor(verts, ind, tex);
 Mesh light(lightVerts, lightInd, tex);

    // Enables the Depth Buffer
    glEnable(GL_DEPTH_TEST);

    // Creates camera object
    Camera camera(800, 600, glm::vec3(0.0f, 0.0f, 2.0f));

    // Main while loop
    while (!glfwWindowShouldClose(window))
    {
        // Specify the color of the background
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        // Clean the back buffer and depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Handles camera inputs
        camera.Inputs(window);
        // Updates and exports the camera matrix to the Vertex Shader


        // Tells OpenGL which Shader Program we want to use
  floor.Draw(shaderProgram, camera);
  light.Draw(lightShader, camera);


        // Swap the back buffer with the front buffer
        glfwSwapBuffers(window);
        // Take care of all GLFW events
        glfwPollEvents();
    }



    // Delete all the objects we've created
    shaderProgram.Delete();
    lightShader.Delete();
    //lightShader.Delete();
    // Delete window before ending the program

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
