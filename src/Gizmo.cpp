#include "Gizmo.h"
#include <glad/glad.h>
#include <vector>

Gizmo::Gizmo() {
    setupGizmo();
}

void Gizmo::setupGizmo() {
    std::vector<float> vertices = {
        // X axis
        0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        // Y axis
        0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        // Z axis
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Gizmo::Draw(Shader& shader, Camera& camera, const glm::vec3& position) {
    shader.use();
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
    
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, 6);
    glBindVertexArray(0);
}

void Gizmo::HandleMouse(GLFWwindow* window, Camera& camera, const glm::vec3& objectPosition, glm::vec3& newObjectPosition)
{
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = camera.GetProjectionMatrix();
    glm::vec4 viewport = glm::vec4(0, 0, camera.width, camera.height);

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !isDragging)
    {
        isDragging = true;
        objectStartPos = objectPosition;

        // Check which axis is selected
        float min_dist = 0.1f;
        selectedAxis = NONE;

        // Project axis lines to screen space and check distance to mouse
        glm::vec3 screenPos = glm::project(objectPosition, view, projection, viewport);
        glm::vec3 screenX = glm::project(objectPosition + glm::vec3(1,0,0), view, projection, viewport);
        glm::vec3 screenY = glm::project(objectPosition + glm::vec3(0,1,0), view, projection, viewport);
        glm::vec3 screenZ = glm::project(objectPosition + glm::vec3(0,0,1), view, projection, viewport);

        // Simplified 2D distance check
        if (glm::distance(glm::vec2(mouseX, camera.height - mouseY), glm::vec2(screenX)) < min_dist * 100) selectedAxis = X;
        else if (glm::distance(glm::vec2(mouseX, camera.height - mouseY), glm::vec2(screenY)) < min_dist * 100) selectedAxis = Y;
        else if (glm::distance(glm::vec2(mouseX, camera.height - mouseY), glm::vec2(screenZ)) < min_dist * 100) selectedAxis = Z;

        dragStartWorldPos = camera.Position; // This is a simplification
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
    {
        isDragging = false;
        selectedAxis = NONE;
    }

    if (isDragging && selectedAxis != NONE)
    {
        glm::vec3 ray_origin = camera.Position;
        glm::vec3 ray_direction = camera.GetRay(window);

        float t;

        // Intersect ray with a plane defined by the selected axis
        glm::vec3 plane_normal;
        switch(selectedAxis) {
            case X: plane_normal = glm::vec3(0,0,1); break; // move on XY plane for X axis drag
            case Y: plane_normal = glm::vec3(0,0,1); break; // move on XY plane for Y axis drag
            case Z: plane_normal = glm::vec3(1,0,0); break; // move on ZY plane for Z axis drag
        }
        
        float denom = glm::dot(plane_normal, ray_direction);
        if (abs(denom) > 1e-6) {
            t = glm::dot(objectStartPos - ray_origin, plane_normal) / denom;
            glm::vec3 intersection_point = ray_origin + t * ray_direction;

            switch(selectedAxis) {
                case X: newObjectPosition.x = intersection_point.x; break;
                case Y: newObjectPosition.y = intersection_point.y; break;
                case Z: newObjectPosition.z = intersection_point.z; break;
            }
        }
    }
}