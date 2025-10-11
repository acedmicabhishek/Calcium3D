#ifndef WATER_H
#define WATER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "Shader.h"
#include "Mesh.h"

class Water {
public:
    Water();
    void Draw(Shader& shader, Camera& camera, glm::mat4 projection, float time, glm::vec3 viewPos);
    bool visible = true;

    // Water properties
    glm::vec3 position = glm::vec3(0.0f, -5.0f, 0.0f);
    float waveSpeed = 1.0f;
    float waveStrength = 0.1f;
    float shininess = 128.0f;
    glm::vec3 waterColor = glm::vec3(0.0f, 0.3f, 0.5f);
    int waveSystem = 0; // 0 for Blinn-Wyvill, 1 for Gerstner


private:
    Mesh waterMesh;
};

#endif