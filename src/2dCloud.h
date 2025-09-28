#ifndef CLOUD_H
#define CLOUD_H

#include "Shader.h"
#include "Texture.h"
#include "Camera.h"
#include <glm/glm.hpp>

class Cloud2D {
public:
    Cloud2D(const char* noiseTexturePath);
    void Draw(Shader& shader, Camera& camera, const glm::mat4& model);

private:
    unsigned int VAO, VBO;
    Texture noiseTex;
};

#endif