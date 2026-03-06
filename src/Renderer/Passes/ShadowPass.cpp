#include "ShadowPass.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Scene.h"
#include "Camera.h"
#include <glad/glad.h>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

ShadowPass::ShadowPass()
{
    m_Name = "ShadowPass";
}

ShadowPass::~ShadowPass()
{
    if (m_DirShadowFBO) glDeleteFramebuffers(1, &m_DirShadowFBO);
    if (m_DirShadowMap) glDeleteTextures(1, &m_DirShadowMap);
    
    if (m_PointShadowFBO) glDeleteFramebuffers(1, &m_PointShadowFBO);
    for (int i = 0; i < 4; i++) {
        if (m_PointShadowMaps[i]) glDeleteTextures(1, &m_PointShadowMaps[i]);
    }
}

void ShadowPass::Init()
{
    
    glGenFramebuffers(1, &m_DirShadowFBO);
    glGenTextures(1, &m_DirShadowMap);
    glBindTexture(GL_TEXTURE_2D, m_DirShadowMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_DirShadowRes, m_DirShadowRes, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    
    glBindFramebuffer(GL_FRAMEBUFFER, m_DirShadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DirShadowMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    
    glGenFramebuffers(1, &m_PointShadowFBO);
    for (int i = 0; i < 4; i++) {
        glGenTextures(1, &m_PointShadowMaps[i]);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_PointShadowMaps[i]);
        for (unsigned int j = 0; j < 6; ++j) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, 0, GL_DEPTH_COMPONENT, 
                         m_PointShadowRes, m_PointShadowRes, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }
}

void ShadowPass::Execute(const RenderContext& context)
{
    if (!context.scene) return;
    
    context.dirShadowMap = m_DirShadowMap;
    for(int i=0; i<4; i++) context.pointShadowCubemaps[i] = m_PointShadowMaps[i];

    if (!context.enableShadows && !context.enablePointShadows) {
        return;
    }

    glEnable(GL_DEPTH_TEST);
    
    
    if (context.enableShadows && context.sunEnabled) {
        glViewport(0, 0, m_DirShadowRes, m_DirShadowRes);
        glBindFramebuffer(GL_FRAMEBUFFER, m_DirShadowFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_FRONT); 
        glEnable(GL_CULL_FACE);

        
        glm::vec3 lightPos = context.sunPosition;
        if (glm::length(lightPos) < 1.0f) lightPos = glm::vec3(0, 100, 0); 
        
        
        float orthoSize = 25.0f;
        glm::mat4 lightProjection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, 1.0f, 100.0f);
        
        
        glm::vec3 target = context.camera ? context.camera->Position : glm::vec3(0.0f);
        glm::mat4 lightView = glm::lookAt(target + normalize(lightPos) * 30.0f, target, glm::vec3(0.0, 1.0, 0.0));
        
        context.lightSpaceMatrix = lightProjection * lightView;

        
        Shader& shadowShader = ResourceManager::GetShader("shadow");
        
        shadowShader.use();
        shadowShader.setMat4("lightSpaceMatrix", context.lightSpaceMatrix);
        
        shadowShader.use();
        shadowShader.setMat4("lightSpaceMatrix", context.lightSpaceMatrix);
        
        
        for (const auto& obj : context.scene->GetObjects()) {
            if (!obj.isActive || obj.meshType == MeshType::Camera) continue;

            shadowShader.use();

            
            glm::mat4 model = context.scene->GetGlobalTransform(&obj - &context.scene->GetObjects()[0]);
            shadowShader.setMat4("model", model);


            
            
            obj.mesh.vao.Bind();
            glDrawElements(GL_TRIANGLES, obj.mesh.indices.size(), GL_UNSIGNED_INT, 0);
            obj.mesh.vao.Unbind();
        }
    }

    
    if (context.enablePointShadows) {
        glViewport(0, 0, m_PointShadowRes, m_PointShadowRes);
        glBindFramebuffer(GL_FRAMEBUFFER, m_PointShadowFBO);
        glDisable(GL_CULL_FACE); 
        
        Shader& pointShadowShader = ResourceManager::GetShader("point_shadow");

        int shadowCasters = 0;
        const auto& pointLights = context.scene->GetPointLights();
        
        for (const auto& light : pointLights) {
            if (!light.enabled || !light.castShadows) continue;
            if (shadowCasters >= 4) break; 

            
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_PointShadowMaps[shadowCasters], 0);
            glClear(GL_DEPTH_BUFFER_BIT);

            float aspect = (float)m_PointShadowRes / (float)m_PointShadowRes;
            float near = 0.5f;
            float far = context.pointShadowFarPlane;
            glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, near, far);
            
            std::vector<glm::mat4> shadowTransforms;
            shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3( 1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0)));
            shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0)));
            shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3( 0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
            shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3( 0.0,-1.0, 0.0), glm::vec3(0.0, 0.0,-1.0)));
            shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3( 0.0, 0.0, 1.0), glm::vec3(0.0,-1.0, 0.0)));
            shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3( 0.0, 0.0,-1.0), glm::vec3(0.0,-1.0, 0.0)));

            pointShadowShader.use();
            for (int j = 0; j < 6; ++j) pointShadowShader.setMat4("shadowMatrices[" + std::to_string(j) + "]", shadowTransforms[j]);
            pointShadowShader.setVec3("lightPos", light.position);
            pointShadowShader.setFloat("far_plane", far);
            
            pointShadowShader.use();
            for (int j = 0; j < 6; ++j) pointShadowShader.setMat4("shadowMatrices[" + std::to_string(j) + "]", shadowTransforms[j]);
            pointShadowShader.setVec3("lightPos", light.position);
            pointShadowShader.setFloat("far_plane", far);

            
            for (const auto& obj : context.scene->GetObjects()) {
                if (!obj.isActive || obj.meshType == MeshType::Camera) continue;

                pointShadowShader.use();
                glm::mat4 model = context.scene->GetGlobalTransform(&obj - &context.scene->GetObjects()[0]);
                pointShadowShader.setMat4("model", model);


                
                obj.mesh.vao.Bind();
                glDrawElements(GL_TRIANGLES, obj.mesh.indices.size(), GL_UNSIGNED_INT, 0);
                obj.mesh.vao.Unbind();
            }
            shadowCasters++;
        }
    }

    
    glDisable(GL_CULL_FACE);
    glBindFramebuffer(GL_FRAMEBUFFER, context.mainFBO);
    glViewport(0, 0, context.width, context.height);
}

void ShadowPass::Resize(int width, int height)
{
    
}
