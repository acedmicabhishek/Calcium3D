#include "Water.h"
#include "ObjectFactory.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Camera.h"

Water::Water() : waterMesh(ObjectFactory::createPlane()) {}

void Water::Draw(Shader& shader, Camera& camera, glm::mat4 projection, float time, glm::vec3 viewPos) {
    if (!visible) return;

    shader.use();
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, glm::vec3(camera.farPlane * 2.0f));
    shader.setMat4("model", model);
    shader.setMat4("view", camera.GetViewMatrix());
    shader.setMat4("projection", projection);
    shader.setFloat("time", time);
    shader.setVec3("viewPos", viewPos);
    shader.setFloat("waveSpeed", waveSpeed);
    shader.setFloat("waveStrength", waveStrength);
    shader.setFloat("shininess", shininess);
    shader.setVec3("waterColor", waterColor);
    shader.setInt("waveSystem", waveSystem);


    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    waterMesh.Draw(shader, camera, glm::vec3(0.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.0f));

    glDisable(GL_BLEND);
}