#include "Renderer.h"
#include <iostream>

void Renderer::Init() {
    glEnable(GL_DEPTH_TEST);
    
}

void Renderer::Shutdown() {
    
}

void Renderer::BeginScene(Camera& camera, const glm::vec4& clearColor) {
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::EndScene() {
    
}

void Renderer::RenderMesh(Mesh& mesh, Shader& shader, const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale) {
    
    
}

void Renderer::RenderScene(Scene& scene, Camera& camera, Shader& shader, float tilingFactor) {
    auto& objects = scene.GetObjects();
    
    shader.use();
    shader.setMat4("view", camera.GetViewMatrix());
    shader.setMat4("projection", camera.GetProjectionMatrix());

    for (size_t i = 0; i < objects.size(); ++i) {
        auto& object = objects[i];
        
        shader.setVec3("material.albedo", object.material.albedo);
        shader.setFloat("material.metallic", object.material.metallic);
        shader.setFloat("material.roughness", object.material.roughness);
        shader.setFloat("material.ao", object.material.ao);
        shader.setFloat("material.shininess", object.material.shininess);
        shader.setBool("material.useTexture", object.material.useTexture);
        
        glm::mat4 globalTransform = scene.GetGlobalTransform(i);
        
        
        glm::mat4 finalMatrix = glm::scale(globalTransform, glm::vec3(tilingFactor));
        object.mesh.Draw(shader, camera, finalMatrix);
    }
}
