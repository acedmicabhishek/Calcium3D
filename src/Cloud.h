#ifndef CLOUD_H
#define CLOUD_H

#include "Shader.h"
#include "Texture.h"
#include "Camera.h"
#include <glm/glm.hpp>

class Cloud {
public:
    Cloud(const char* noiseTexturePath);
    void Draw(Shader& shader, Camera& camera);

private:
    unsigned int VAO, VBO;
    Texture noiseTex;
};

#endif