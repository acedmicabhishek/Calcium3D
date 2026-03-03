#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <memory>


class Scene;
class Camera;
class Cloud2D;
class VolumetricCloud;

struct RenderContext {
    Scene* scene = nullptr;
    Camera* camera = nullptr;
    Cloud2D* cloud2d = nullptr;
    VolumetricCloud* volCloud = nullptr;
    
    
    float time = 0.0f;
    float deltaTime = 0.0f;
    float timeOfDay = 12.0f;
    
    
    glm::vec3 sunPosition = glm::vec3(0.0f);
    glm::vec4 sunColor = glm::vec4(1.0f);
    float sunIntensity = 1.0f;
    bool sunEnabled = true;
    float sunBloom = 0.03f;
    
    glm::vec3 moonPosition = glm::vec3(0.0f);
    glm::vec4 moonColor = glm::vec4(0.8f, 0.8f, 0.9f, 1.0f);
    float moonIntensity = 0.5f;
    bool moonEnabled = true;
    float moonBloom = 0.02f;
    
    
    unsigned int mainFBO = 0;
    unsigned int outputTexture = 0;
    
    
    int width = 0;
    int height = 0;
    
    
    bool showClouds = false;
    int cloudMode = 0; 
    float cloudHeight = 50.0f;
    
    
    float cloudDensity = 0.5f;
    float cloudCover = 0.5f;
    
    bool wireframe = false;
    bool showSkybox = true;
    bool showGradientSky = false;
    float globalTilingFactor = 1.0f;
    
    
    int msaaSamples = 0;
    bool msaaSkyPass = true;
    bool msaaGeometryPass = true;
    bool msaaTransparencyPass = true;
    
    bool renderEditorObjects = false;
};
