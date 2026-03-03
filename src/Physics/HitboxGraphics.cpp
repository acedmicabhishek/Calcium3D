#include "HitboxGraphics.h"
#include "../Core/ResourceManager.h"
#include <glad/glad.h>

bool HitboxGraphics::ShowHitboxes = false;
unsigned int HitboxGraphics::m_VAO = 0;
unsigned int HitboxGraphics::m_VBO = 0;
unsigned int HitboxGraphics::m_EBO = 0;
unsigned int HitboxGraphics::m_DynVAO = 0;
unsigned int HitboxGraphics::m_DynVBO = 0;

void HitboxGraphics::Init() {
    if (m_VAO != 0) return;

    float vertices[] = {
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f, -0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f
    };

    unsigned int indices[] = {
        0, 1, 1, 3, 3, 2, 2, 0, 
        4, 5, 5, 7, 7, 6, 6, 4, 
        0, 4, 1, 5, 2, 6, 3, 7  
    };

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glGenVertexArrays(1, &m_DynVAO);
    glGenBuffers(1, &m_DynVBO);
    glBindVertexArray(m_DynVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_DynVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 24, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void HitboxGraphics::Render(Scene& scene, Camera& camera) {
    if (!ShowHitboxes) return;

    Shader& shader = ResourceManager::GetShader("hitbox");
    shader.use();
    shader.setMat4("view", camera.GetViewMatrix());
    shader.setMat4("projection", camera.GetProjectionMatrix());

    glBindVertexArray(m_VAO);

    for (auto& obj : scene.GetObjects()) {
        if (!obj.isActive) continue;
        
        glm::mat4 modelOBB = glm::translate(glm::mat4(1.0f), obj.position) * 
                             glm::mat4_cast(obj.rotation) * 
                             glm::scale(glm::mat4(1.0f), obj.scale);
        
        
        glm::vec3 localCenter = (obj.collider.min + obj.collider.max) * 0.5f;
        glm::vec3 localSize = obj.collider.max - obj.collider.min;
        
        modelOBB = glm::translate(modelOBB, localCenter) * glm::scale(glm::mat4(1.0f), localSize);

        shader.setMat4("model", modelOBB);
        shader.setVec3("color", glm::vec3(1.0f, 0.0f, 0.0f)); 
        glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);

        
        AABB worldAABB = PhysicsEngine::GetTransformedAABB(obj.collider, obj.position, obj.rotation, obj.scale);
        glm::vec3 aabbCenter = (worldAABB.min + worldAABB.max) * 0.5f;
        glm::vec3 aabbSize = worldAABB.max - worldAABB.min;

        glm::mat4 modelAABB = glm::translate(glm::mat4(1.0f), aabbCenter) * glm::scale(glm::mat4(1.0f), aabbSize);
        shader.setMat4("model", modelAABB);
        shader.setVec3("color", glm::vec3(0.0f, 1.0f, 0.0f)); 
        glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);

        
        if (PhysicsEngine::GlobalCOMEnabled) {
            glm::vec3 worldCOM = obj.position + obj.rotation * obj.centerOfMassOffset;
            glm::mat4 modelCOM = glm::translate(glm::mat4(1.0f), worldCOM) * glm::scale(glm::mat4(1.0f), glm::vec3(0.05f));
            shader.setMat4("model", modelCOM);
            shader.setVec3("color", glm::vec3(1.0f, 1.0f, 0.0f)); 
            glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
        }
    }
    glBindVertexArray(0);

    
    glBindVertexArray(m_DynVAO);
    for (auto& obj : scene.GetObjects()) {
        if (!obj.isActive) continue;
        if (obj.hasCamera && obj.camera.enabled) {
            float fov = glm::radians(obj.camera.fov);
            float nearP = obj.camera.nearPlane;
            float farP = glm::min(obj.camera.farPlane, 20.0f);
            float aspect = (float)obj.camera.resolutionX / (float)obj.camera.resolutionY;
            float nearH = 2.0f * glm::tan(fov * 0.5f) * nearP;
            float nearW = nearH * aspect;
            float farH = 2.0f * glm::tan(fov * 0.5f) * farP;
            float farW = farH * aspect;

            glm::vec3 ntl(-nearW/2, nearH/2, -nearP), ntr(nearW/2, nearH/2, -nearP);
            glm::vec3 nbl(-nearW/2, -nearH/2, -nearP), nbr(nearW/2, -nearH/2, -nearP);
            glm::vec3 ftl(-farW/2, farH/2, -farP), ftr(farW/2, farH/2, -farP);
            glm::vec3 fbl(-farW/2, -farH/2, -farP), fbr(farW/2, -farH/2, -farP);

            std::vector<glm::vec3> lines = {
                ntl,ntr, ntr,nbr, nbr,nbl, nbl,ntl, 
                ftl,ftr, ftr,fbr, fbr,fbl, fbl,ftl, 
                ntl,ftl, ntr,ftr, nbl,fbl, nbr,fbr  
            };

            glBindBuffer(GL_ARRAY_BUFFER, m_DynVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * lines.size(), lines.data());

            glm::mat4 modelMat = glm::translate(glm::mat4(1.0f), obj.position) * glm::mat4_cast(obj.rotation);
            shader.setMat4("model", modelMat);
            shader.setVec3("color", glm::vec3(0.8f, 0.8f, 1.0f)); 
            glDrawArrays(GL_LINES, 0, 24);
        }
    }
    glBindVertexArray(0);
}

void HitboxGraphics::Cleanup() {
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);
    glDeleteVertexArrays(1, &m_DynVAO);
    glDeleteBuffers(1, &m_DynVBO);
}
