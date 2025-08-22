#ifndef GIZMO_H
#define GIZMO_H

#include "Shader.h"
#include "Camera.h"
#include "Editor.h"
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

enum GizmoAxis { NONE, X, Y, Z };

class Gizmo {
public:
    Gizmo();
    void Draw(Shader& shader, Camera& camera, const glm::vec3& position);
    void HandleMouse(GLFWwindow* window, Camera& camera, const glm::vec3& objectPosition, glm::vec3& newObjectPosition);
private:
    unsigned int VAO, VBO;
    void setupGizmo();
    GizmoAxis selectedAxis = NONE;
    bool isDragging = false;
    glm::vec3 dragStartWorldPos;
    glm::vec3 objectStartPos;
};

#endif