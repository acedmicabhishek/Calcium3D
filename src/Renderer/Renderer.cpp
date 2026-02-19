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

    for (auto& object : objects) {
        
        glm::vec3 finalScale = object.scale * tilingFactor;
        object.mesh.Draw(shader, camera, object.position, object.rotation, finalScale);
    }
}
