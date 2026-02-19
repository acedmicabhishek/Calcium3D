#include "2dCloud.h"
#include <glad/glad.h>
#include <vector>

Cloud2D::Cloud2D() {
    // A fullscreen quad
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

#include <GLFW/glfw3.h>

void Cloud2D::Draw(Shader& shader, Camera& camera, const glm::mat4& model) {
    shader.use();
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(camera.GetProjectionMatrix()));
    glUniform3f(glGetUniformLocation(shader.ID, "viewPos"), camera.Position.x, camera.Position.y, camera.Position.z);

    glUniform1f(glGetUniformLocation(shader.ID, "u_time"), (float)glfwGetTime());
    glUniform2f(glGetUniformLocation(shader.ID, "u_resolution"), (float)camera.width, (float)camera.height);
    glUniform3f(glGetUniformLocation(shader.ID, "u_cloudColor"), cloudColor.x, cloudColor.y, cloudColor.z);
    glUniform1f(glGetUniformLocation(shader.ID, "u_cloudCover"), cloudCover);
    glUniform1f(glGetUniformLocation(shader.ID, "u_cloudSpeed"), cloudSpeed);
    glUniform1f(glGetUniformLocation(shader.ID, "u_tiling"), tiling);
    glUniform1f(glGetUniformLocation(shader.ID, "u_density"), density);
    glUniform1f(glGetUniformLocation(shader.ID, "u_cloudSize"), cloudSize);
    glUniform1f(glGetUniformLocation(shader.ID, "u_randomness"), randomness);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}