#include "VolumetricCloud.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

VolumetricCloud::VolumetricCloud() {
    setupCloud();
}

VolumetricCloud::~VolumetricCloud() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void VolumetricCloud::setupCloud() {
    float quadVertices[] = {
        // positions        // texture Coords
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,

        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
}

void VolumetricCloud::Draw(Shader& shader, Camera& camera, float cloudHeight, float farPlane) {
    shader.use();
    glm::mat4 projection = camera.GetProjectionMatrix();
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 invProjection = glm::inverse(projection);
    glm::mat4 invView = glm::inverse(view);

    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "invProjection"), 1, GL_FALSE, glm::value_ptr(invProjection));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "invView"), 1, GL_FALSE, glm::value_ptr(invView));
    glUniform3fv(glGetUniformLocation(shader.ID, "cameraPosition"), 1, glm::value_ptr(camera.Position));
    glUniform1f(glGetUniformLocation(shader.ID, "time"), glfwGetTime());
    glUniform1f(glGetUniformLocation(shader.ID, "cloudHeight"), cloudHeight);
    glUniform1f(glGetUniformLocation(shader.ID, "farPlane"), farPlane);
    glUniform1f(glGetUniformLocation(shader.ID, "density"), density);
    glUniform1f(glGetUniformLocation(shader.ID, "stepSize"), stepSize);
    glUniform1f(glGetUniformLocation(shader.ID, "cloudCover"), cloudCover);
    glUniform1f(glGetUniformLocation(shader.ID, "speed"), speed);
    glUniform1i(glGetUniformLocation(shader.ID, "quality"), quality);
    glUniform1f(glGetUniformLocation(shader.ID, "detail"), detail);


    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}