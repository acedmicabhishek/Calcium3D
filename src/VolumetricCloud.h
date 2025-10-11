#ifndef VOLUMETRIC_CLOUD_H
#define VOLUMETRIC_CLOUD_H

#include "Shader.h"
#include "Camera.h"
#include <glm/glm.hpp>

class VolumetricCloud {
public:
    VolumetricCloud();
    ~VolumetricCloud();
    void Draw(Shader& shader, Camera& camera, float cloudHeight, float farPlane);

    float density = 0.5f;
    float stepSize = 0.05f;
    float cloudCover = 0.5f;
    float speed = 0.05f;
    int quality = 0; // 0 for Performance, 1 for Quality
    float detail = 5.0f;

private:
    unsigned int VAO, VBO;
    void setupCloud();
};

#endif