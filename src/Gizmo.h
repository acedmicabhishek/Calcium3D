#ifndef GIZMO_H
#define GIZMO_H

#include "Shader.h"
#include "Camera.h"
#include "Editor.h"
#include <glm/glm.hpp>

class Gizmo {
public:
    Gizmo();
    void Draw(Shader& shader, Camera& camera, const glm::vec3& position);

private:
    unsigned int VAO, VBO;
    void setupGizmo();
};

#endif