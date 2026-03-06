#include "SkyPass.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Scene.h"
#include "Camera.h"
#include <glad/glad.h>
#include <vector>
#include <string>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <stb/stb_image.h>
#include "../Tools/Profiler/Profiler.h"
#include "../Tools/Profiler/GpuProfiler.h"

SkyPass::SkyPass()
{
    m_Name = "SkyPass";
}


static unsigned int s_SkyVAO = 0;
static unsigned int s_SkyVBO = 0;
static unsigned int s_SkyTexture = 0;

void SkyPass::Init()
{
    
    float skyboxVertices[] =
    {
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f
    };

    unsigned int skyboxIndices[] =
    {
        1, 2, 6, 6, 5, 1,
        0, 4, 7, 7, 3, 0,
        4, 5, 6, 6, 7, 4,
        0, 3, 2, 2, 1, 0,
        0, 1, 5, 5, 4, 0,
        3, 7, 6, 6, 2, 3
    };

    if (s_SkyVAO == 0) {
        glGenVertexArrays(1, &s_SkyVAO);
        glGenBuffers(1, &s_SkyVBO);
        unsigned int skyEBO;
        glGenBuffers(1, &skyEBO);
        
        glBindVertexArray(s_SkyVAO);
        glBindBuffer(GL_ARRAY_BUFFER, s_SkyVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
        
    }

    
    if (s_SkyTexture == 0) {
        glGenTextures(1, &s_SkyTexture);
        glBindTexture(GL_TEXTURE_CUBE_MAP, s_SkyTexture);

        std::vector<std::string> faces = {
            "../Resource/cubemap/px.png", "../Resource/cubemap/nx.png",
            "../Resource/cubemap/py.png", "../Resource/cubemap/ny.png",
            "../Resource/cubemap/pz.png", "../Resource/cubemap/nz.png"
        };
        
        int width, height, nrChannels;
        for (unsigned int i = 0; i < faces.size(); i++) {
            stbi_set_flip_vertically_on_load(false);
            std::string resolved = ResourceManager::ResolvePath(faces[i]);
            unsigned char* data = stbi_load(resolved.c_str(), &width, &height, &nrChannels, 0);
            if (data) {
                GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);
                stbi_image_free(data);
            } else {
                std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            }
        }
        
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }
    
    
    
}

void SkyPass::Reload() {
    if (s_SkyTexture) {
        glDeleteTextures(1, &s_SkyTexture);
        s_SkyTexture = 0;
    }
    
    glGenTextures(1, &s_SkyTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, s_SkyTexture);

    std::vector<std::string> faces = {
        "../Resource/cubemap/px.png", "../Resource/cubemap/nx.png",
        "../Resource/cubemap/py.png", "../Resource/cubemap/ny.png",
        "../Resource/cubemap/pz.png", "../Resource/cubemap/nz.png"
    };
    
    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        stbi_set_flip_vertically_on_load(false);
        std::string resolved = ResourceManager::ResolvePath(faces[i]);
        unsigned char* data = stbi_load(resolved.c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
        }
    }
    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    
    std::cout << "Skybox Reloaded." << std::endl;
}

unsigned int SkyPass::GetCubemapTexture() { return s_SkyTexture; }

void SkyPass::Execute(const RenderContext& context)
{
    PROFILE_SCOPE("SkyPass");
    GPU_PROFILE_SCOPE("SkyPass");
    bool showGradient = context.showGradientSky;
    bool showCubemap = context.showSkybox && !context.showGradientSky;
    
    if (!showGradient && !showCubemap) return;
    
    
    if (context.msaaSamples > 0) {
        if (context.msaaSkyPass)
            glEnable(GL_MULTISAMPLE);
        else
            glDisable(GL_MULTISAMPLE);
    }
    
    if (showGradient) {
        
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE); 
        
        Shader& shader = ResourceManager::GetShader("gradientSky");
        shader.use();
        

        
        glm::mat4 view = context.camera->GetViewMatrix();
        glm::mat4 projection = context.camera->GetProjectionMatrix();
        
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        
        float time = context.timeOfDay;
        glUniform1f(glGetUniformLocation(shader.ID, "u_time"), time);
        
        
        float angle = (time - 6.0f) / 24.0f * 2.0f * glm::pi<float>();
        float dayNightCycle = sin(angle) * 0.5f + 0.5f;
        
        glm::vec3 dayTopColor = glm::vec3(0.5f, 0.7f, 1.0f);
        glm::vec3 nightTopColor = glm::vec3(0.0f, 0.0f, 0.1f);
        glm::vec3 dayBottomColor = glm::vec3(1.0f, 1.0f, 1.0f);
        glm::vec3 nightBottomColor = glm::vec3(0.1f, 0.1f, 0.2f);
        
        glm::vec3 topColor = glm::mix(nightTopColor, dayTopColor, dayNightCycle);
        glm::vec3 bottomColor = glm::mix(nightBottomColor, dayBottomColor, dayNightCycle);
        
        glUniform3f(glGetUniformLocation(shader.ID, "topColor"), topColor.x, topColor.y, topColor.z);
        glUniform3f(glGetUniformLocation(shader.ID, "bottomColor"), bottomColor.x, bottomColor.y, bottomColor.z);
        
        
        glm::vec3 DynamicSunPos = glm::vec3(cos(angle) * 10.0f, sin(angle) * 10.0f, 0.0f);
        glm::vec3 DynamicMoonPos = glm::vec3(-DynamicSunPos.x, -DynamicSunPos.y, 0.0f);
        
        glUniform3f(glGetUniformLocation(shader.ID, "DynamicSunPos"), DynamicSunPos.x, DynamicSunPos.y, DynamicSunPos.z);
        glUniform3f(glGetUniformLocation(shader.ID, "DynamicMoonPos"), DynamicMoonPos.x, DynamicMoonPos.y, DynamicMoonPos.z);
        
        glUniform1f(glGetUniformLocation(shader.ID, "sunBloom"), context.sunBloom);
        glUniform1f(glGetUniformLocation(shader.ID, "moonBloom"), context.moonBloom);
        
        glUniform3f(glGetUniformLocation(shader.ID, "sunColor"), 1.0f, 1.0f, 0.0f);
        glUniform3f(glGetUniformLocation(shader.ID, "moonColor"), 0.8f, 0.8f, 0.9f);
        
        glBindVertexArray(s_SkyVAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        
        glEnable(GL_DEPTH_TEST);
    } else {
        
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
        glDisable(GL_CULL_FACE);
        
        Shader& shader = ResourceManager::GetShader("skybox");
        shader.use();
        
        glm::mat4 view = glm::mat4(glm::mat3(context.camera->GetViewMatrix())); 
        glm::mat4 projection = context.camera->GetProjectionMatrix();
        
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        
        glBindVertexArray(s_SkyVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, s_SkyTexture);
        shader.setInt("skybox", 0);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        
        glEnable(GL_CULL_FACE);
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
    }
}
