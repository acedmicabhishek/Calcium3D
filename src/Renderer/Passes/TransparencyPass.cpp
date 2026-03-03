#include "TransparencyPass.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Scene.h"
#include "Camera.h"
#include "2dCloud.h"
#include "VolumetricCloud.h" 
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

TransparencyPass::TransparencyPass()
{
    m_Name = "TransparencyPass";
}

void TransparencyPass::Init()
{
    
}

void TransparencyPass::Execute(const RenderContext& context)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    
    if (context.showClouds) {
        
        if (context.msaaSamples > 0) {
            if (context.msaaTransparencyPass)
                glEnable(GL_MULTISAMPLE);
            else
                glDisable(GL_MULTISAMPLE);
        }
        
        if (context.cloudMode == 0 && context.cloud2d) {
            
            Shader& cloud2dShader = ResourceManager::GetShader("cloud2d");
            
            
            context.cloud2d->cloudCover = context.cloudCover;
            context.cloud2d->density = context.cloudDensity;
            

            glm::mat4 cloud2dModel = glm::mat4(1.0f);
            cloud2dModel = glm::translate(cloud2dModel, glm::vec3(0.0f, context.cloudHeight, 0.0f));
            cloud2dModel = glm::rotate(cloud2dModel, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            cloud2dModel = glm::scale(cloud2dModel, glm::vec3(context.camera->farPlane * 2.0f));
            
            context.cloud2d->Draw(cloud2dShader, *context.camera, cloud2dModel);
        } else if (context.cloudMode == 1 && context.volCloud) {
            
            Shader& volCloudShader = ResourceManager::GetShader("volumetric_cloud");
            
            context.volCloud->cloudCover = context.cloudCover;
            
            
            glDisable(GL_DEPTH_TEST);
            context.volCloud->Draw(volCloudShader, *context.camera, context.cloudHeight, context.camera->farPlane);
            glEnable(GL_DEPTH_TEST);
        }
    }

    
    if (context.scene && context.camera) {
        Shader& waterShader = ResourceManager::GetShader("water");
        
        auto& objects = context.scene->GetObjects();
        for (int i = 0; i < objects.size(); ++i) {
            auto& obj = objects[i];
            if (obj.hasWater && obj.isActive) {
                waterShader.use();
                
                glm::mat4 model = context.scene->GetGlobalTransform(i);
                waterShader.setMat4("model", model);
                waterShader.setMat4("view", context.camera->GetViewMatrix());
                glm::mat4 projection = context.camera->GetProjectionMatrix();
                waterShader.setMat4("projection", projection);
                
                waterShader.setFloat("time", context.time);
                waterShader.setVec3("viewPos", context.camera->Position);
                
                waterShader.setFloat("waveSpeed", obj.water.waveSpeed);
                waterShader.setFloat("waveStrength", obj.water.waveStrength);
                waterShader.setFloat("shininess", obj.water.shininess);
                waterShader.setVec3("waterColor", obj.water.waterColor);
                waterShader.setInt("waveSystem", obj.water.waveSystem);
                waterShader.setFloat("tiling", obj.water.tiling);
                waterShader.setFloat("surfaceHeight", obj.water.surfaceHeight);
                
                obj.mesh.Draw(waterShader, *context.camera, model);
            }
        }
    }
    
    glDisable(GL_BLEND);
}
