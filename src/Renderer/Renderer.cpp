#include "Renderer.h"
#include <iostream>
#include "Core/ResourceManager.h"
#include "Core/Logger.h"
#include "VideoPlayer.h"
#include "AudioEngine/AudioEngine.h"

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

void Renderer::RenderScene(Scene& scene, Camera& camera, Shader& shader, float tilingFactor, bool renderEditorObjects, float dt, float time, int renderLayer) {
    auto& objects = scene.GetObjects();
    
    shader.use();
    shader.setMat4("view", camera.GetViewMatrix());
    shader.setMat4("projection", camera.GetProjectionMatrix());
    shader.setFloat("time", time);
    shader.setFloat("iTime", time);
    shader.setFloat("deltaTime", dt);

    for (size_t i = 0; i < objects.size(); ++i) {
        auto& object = objects[i];
        if (!object.isActive) continue;
        if (!renderEditorObjects && object.meshType == MeshType::Camera) continue;
        
        
        if (renderLayer == 1 && object.material.isTransparent) continue; 
        if (renderLayer == 2 && !object.material.isTransparent) continue; 
        
        Shader* activeShader = &shader;
        if (!object.material.customShaderName.empty()) {
            if (!ResourceManager::HasShader(object.material.customShaderName)) {
                
                std::string sName = object.material.customShaderName;
                std::string vFile = sName + ".vert";
                std::string fFile = sName + ".frag";
                
                
                std::string resolvedV = ResourceManager::ResolvePath(vFile);
                if (!std::filesystem::exists(resolvedV)) {
                    vFile = "default.vert"; 
                }
                
                try {
                    ResourceManager::LoadShader(sName, vFile.c_str(), fFile.c_str());
                    Logger::AddLog("Auto-loaded custom shader: %s", sName.c_str());
                } catch (...) {
                    
                }
            }
            
            if (ResourceManager::HasShader(object.material.customShaderName)) {
                activeShader = &ResourceManager::GetShader(object.material.customShaderName);
            }
        } else if (object.isAnimating && !object.boneMatrices.empty() && ResourceManager::HasShader("skeletal")) {
            activeShader = &ResourceManager::GetShader("skeletal");
        }

        if (!activeShader) {
            continue;
        }

        activeShader->use();
        activeShader->setMat4("view", camera.GetViewMatrix());
        activeShader->setMat4("projection", camera.GetProjectionMatrix());
        activeShader->setFloat("time", time);
        activeShader->setFloat("iTime", time);
        activeShader->setFloat("deltaTime", dt);
        activeShader->setFloat("intensity", 1.0f); 
        
        glm::vec3 finalAlbedo = object.material.albedo;
        if (object.hasScreen && (object.screen.type == ScreenType::Image || object.screen.type == ScreenType::Video)) {
            finalAlbedo *= object.screen.brightness;
        }
        
        activeShader->setVec3("material.albedo", finalAlbedo);
        activeShader->setFloat("material.metallic", object.material.metallic);
        activeShader->setFloat("material.roughness", object.material.roughness);
        activeShader->setFloat("material.ao", object.material.ao);
        activeShader->setFloat("material.shininess", object.material.shininess);
        activeShader->setBool("material.useTexture", object.material.useTexture);
        
        glm::mat4 globalTransform = scene.GetGlobalTransform(i);
        
        
        glm::mat4 finalMatrix = glm::scale(globalTransform, glm::vec3(tilingFactor));
        
        for (int j = 0; j < (int)object.boneMatrices.size() && j < 100; j++) {
            activeShader->setMat4("finalBonesMatrices[" + std::to_string(j) + "]", object.boneMatrices[j]);
        }
        
        unsigned int texOverride = 0;
        if (object.hasScreen && object.screen.enabled) {
            if (object.screen.type == ScreenType::CameraFeed && object.screen.targetCameraIndex != -1) {
                auto& camObjs = scene.GetObjects();
                if (object.screen.targetCameraIndex < (int)camObjs.size()) {
                    texOverride = camObjs[object.screen.targetCameraIndex].camera.renderTexture;
                }
            } else if (object.screen.type == ScreenType::Image && !object.screen.filePath.empty()) {
                
                if (ResourceManager::HasTexture(object.screen.filePath)) {
                    texOverride = ResourceManager::GetTexture(object.screen.filePath).ID;
                } else if (!object.screen.filePath.empty()) {
                    ResourceManager::LoadTexture(object.screen.filePath, object.screen.filePath.c_str(), "diffuse", 0);
                    if (ResourceManager::HasTexture(object.screen.filePath)) {
                        texOverride = ResourceManager::GetTexture(object.screen.filePath).ID;
                    }
                }
            } else if (object.screen.type == ScreenType::Video && !object.screen.filePath.empty()) {
                if (!object.screen.videoPlayerHandle) {
                    VideoPlayer* vp = new VideoPlayer();
                    if (vp->Open(object.screen.filePath)) {
                        object.screen.videoPlayerHandle = std::shared_ptr<void>(vp, [](void* p) {
                            delete static_cast<VideoPlayer*>(p);
                        });
                        vp->Update(0.0f); 
                    } else {
                        delete vp;
                        object.screen.videoPlayerHandle.reset();
                    }
                    
                    if (object.screen.videoPlayerHandle) {
                        
                        std::string audioPath = object.screen.filePath + "_audio.wav";
                        object.hasAudio = true;
                        if (object.audio.filePath != audioPath) {
                            object.audio.filePath = audioPath;
                            object.audio.playing = !object.screen.videoPaused;
                            object.audio.playOnAwake = true;
                            if (object.audio.playing) {
                                AudioEngine::PlayObjectAudio(object);
                            }
                        }
                    }
                }
                if (object.screen.videoPlayerHandle) {
                    VideoPlayer* vp = static_cast<VideoPlayer*>(object.screen.videoPlayerHandle.get());
                    vp->SetLooping(object.screen.videoLoop);
                    
                    if (!object.screen.videoPaused) {
                        vp->Update(dt * object.screen.videoPlaybackSpeed);
                    }
                    texOverride = vp->GetTextureID();
                    
                    
                    if (object.hasAudio) {
                        object.audio.looping = object.screen.videoLoop;
                        object.audio.volume = object.screen.videoVolume;
                        object.audio.pitch = object.screen.videoPlaybackSpeed;
                        
                        if (object.screen.videoPaused && object.audio.playing) {
                            object.audio.playing = false;
                            AudioEngine::StopObjectAudio(object);
                        } else if (!object.screen.videoPaused && !object.audio.playing) {
                            object.audio.playing = true;
                            AudioEngine::PlayObjectAudio(object);
                        }
                    }
                    
                    
                    finalMatrix = glm::scale(finalMatrix, glm::vec3(1.0f, -1.0f, 1.0f));

                    
                    if (object.screen.videoKeepAspect && vp->GetWidth() > 0 && vp->GetHeight() > 0) {
                        float aspect = (float)vp->GetWidth() / (float)vp->GetHeight();
                        
                        finalMatrix = glm::scale(finalMatrix, glm::vec3(aspect, 1.0f, 1.0f));
                    }
                }
            }
        }
        
        object.mesh.Draw(*activeShader, camera, finalMatrix, texOverride, object.boneMatrices);
    }
}
