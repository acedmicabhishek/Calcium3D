#ifndef CLOUD_H
#define CLOUD_H

#include "Shader.h"
#include "Camera.h"
#include <glm/glm.hpp>

class Cloud2D {
public:
    Cloud2D();
    void Draw(Shader& shader, Camera& camera, const glm::mat4& model);

    glm::vec3 cloudColor = glm::vec3(1.0f);
    float cloudCover = 0.5f;
    float cloudSpeed = 0.1f;
    float tiling = 1.0f;
    float density = 1.0f;
    float cloudSize = 2.0f;
    float randomness = 0.5f;

private:
    unsigned int VAO, VBO;
};

#endif