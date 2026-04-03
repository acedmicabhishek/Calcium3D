#include "C3D.h"
#include "../Scene/Scene.h"
#include "../Scene/SceneManager.h"
#include "../Physics/PhysicsEngine.h"
#include "../AudioEngine/AudioEngine.h"
#include "../Core/InputManager.h"
#include "../Core/Logger.h"
#include "../Renderer/VideoPlayer.h"
#include "../Core/Application.h"
#include "../Renderer/Renderer.h"
#include "../UI/UICreationEngine.h"
#include "../Renderer/SDFGenerator.h"
#include <filesystem>
#include <algorithm>
#include <random>

namespace fs = std::filesystem;

namespace C3D {


namespace Object {
    void SetPosition(GameObject* obj, const glm::vec3& pos) { if(obj) obj->position = pos; }
    glm::vec3 GetPosition(GameObject* obj) { return obj ? obj->position : glm::vec3(0.0f); }
    void SetRotation(GameObject* obj, const glm::quat& rot) { if(obj) obj->rotation = rot; }
    void SetRotationEuler(GameObject* obj, const glm::vec3& euler) { if(obj) obj->rotation = glm::quat(glm::radians(euler)); }
    glm::quat GetRotation(GameObject* obj) { return obj ? obj->rotation : glm::quat(1,0,0,0); }
    void SetScale(GameObject* obj, const glm::vec3& scale) { if(obj) obj->scale = scale; }
    glm::vec3 GetScale(GameObject* obj) { return obj ? obj->scale : glm::vec3(1.0f); }
    void SetActive(GameObject* obj, bool active) { if(obj) obj->isActive = active; }
    bool IsActive(GameObject* obj) { return obj ? obj->isActive : false; }
    const std::string& GetName(GameObject* obj) { static std::string empty = ""; return obj ? obj->name : empty; }
    void SetName(GameObject* obj, const std::string& name) { if(obj) obj->name = name; }
    const std::string& GetTag(GameObject* obj) { static std::string empty = ""; return obj ? obj->tag : empty; }
    void SetTag(GameObject* obj, const std::string& tag) { if(obj) obj->tag = tag; }
    void SetParent(GameObject* obj, int parentIndex) { if(obj) obj->parentIndex = parentIndex; }
    int GetParentIndex(GameObject* obj) { return obj ? obj->parentIndex : -1; }
    int GetCount() { if(auto s = SceneManager::Get().GetActiveScene()) return (int)s->GetObjects().size(); return 0; }
    void Duplicate(int index) { if(auto s = SceneManager::Get().GetActiveScene()) s->DuplicateObjectTree(index); }
    void Destroy(int index) { if(auto s = SceneManager::Get().GetActiveScene()) s->RemoveObject(index); }
}


namespace Physics {
    void AddForce(GameObject* obj, const glm::vec3& force) { if(obj) obj->velocity += force / (obj->mass > 0.0f ? obj->mass : 1.0f); }
    void AddImpulse(GameObject* obj, const glm::vec3& impulse) { if(obj) obj->velocity += impulse; }
    void AddTorque(GameObject* obj, const glm::vec3& torque) { if(obj) obj->angularVelocity += torque; }
    void SetVelocity(GameObject* obj, const glm::vec3& velocity) { if(obj) obj->velocity = velocity; }
    glm::vec3 GetVelocity(GameObject* obj) { return obj ? obj->velocity : glm::vec3(0.0f); }
    void SetAngularVelocity(GameObject* obj, const glm::vec3& angularVelocity) { if(obj) obj->angularVelocity = angularVelocity; }
    glm::vec3 GetAngularVelocity(GameObject* obj) { return obj ? obj->angularVelocity : glm::vec3(0.0f); }
    void SetMass(GameObject* obj, float mass) { if(obj) obj->mass = mass; }
    float GetMass(GameObject* obj) { return obj ? obj->mass : 1.0f; }
    void SetFriction(GameObject* obj, float friction) { if(obj) obj->friction = friction; }
    void SetBounciness(GameObject* obj, float restitution) { if(obj) obj->restitution = restitution; }
    void SetGravityEnabled(GameObject* obj, bool enabled) { if(obj) obj->useGravity = enabled; }
    void SetCollisionEnabled(GameObject* obj, bool enabled) { if(obj) obj->enableCollision = enabled; }
    void SetStatic(GameObject* obj, bool isStatic) { if(obj) obj->isStatic = isStatic; }
    void SetTrigger(GameObject* obj, bool isTrigger) { if(obj) obj->isTrigger = isTrigger; }

    bool Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, RaycastHit& outHit) {
        if(auto s = SceneManager::Get().GetActiveScene()) {
            PhysicsEngine::RaycastHit internalHit;
            bool hit = PhysicsEngine::Raycast(origin, direction, maxDistance, s->GetObjects(), internalHit);
            if (hit) {
                outHit.object = internalHit.object;
                outHit.point = internalHit.point;
                outHit.normal = internalHit.normal;
                outHit.distance = internalHit.distance;
            }
            return hit;
        }
        return false;
    }

    namespace Global {
        void SetGravity(const glm::vec3& gravity) { PhysicsEngine::Gravity = gravity; }
        void SetAirResistance(float resistance) { PhysicsEngine::GlobalAirResistance = resistance; }
        void SetSubSteps(int subSteps) { PhysicsEngine::SubSteps = subSteps; }
        void SetPhysicsEnabled(bool enabled) { PhysicsEngine::GlobalPhysicsEnabled = enabled; }
    }
}


namespace Audio {
    void Play(GameObject* obj) { if(obj && obj->hasAudio) AudioEngine::PlayObjectAudio(*obj); }
    void Stop(GameObject* obj) { if(obj && obj->hasAudio) AudioEngine::StopObjectAudio(*obj); }
    void SetClip(GameObject* obj, const std::string& path) { if(obj && obj->hasAudio) obj->audio.filePath = path; }
    void SetVolume(GameObject* obj, float volume) { if(obj && obj->hasAudio) obj->audio.volume = volume; }
    void SetPitch(GameObject* obj, float pitch) { if(obj && obj->hasAudio) obj->audio.pitch = pitch; }
    void SetLooping(GameObject* obj, bool loop) { if(obj && obj->hasAudio) obj->audio.looping = loop; }
    void SetSpatial(GameObject* obj, bool spatial) { if(obj && obj->hasAudio) obj->audio.type = spatial ? AudioType::Directional : AudioType::Ambience; }
    void SetDistances(GameObject* obj, float minDistance, float maxDistance) {
        if(obj && obj->hasAudio) { obj->audio.minDistance = minDistance; obj->audio.maxDistance = maxDistance; }
    }
    void SetDopplerFactor(GameObject* obj, float factor) { if(obj && obj->hasAudio) obj->audio.dopplerFactor = factor; }
    void SetReverbEnabled(GameObject* obj, bool enabled) { if(obj && obj->hasAudio) obj->audio.enableReverb = enabled; }
    void SetOcclusionEnabled(GameObject* obj, bool enabled) { if(obj && obj->hasAudio) obj->audio.enableOcclusion = enabled; }
}


namespace Graphics {
    void SetAlbedo(GameObject* obj, const glm::vec3& color) { if(obj) obj->material.albedo = color; }
    void SetMetallic(GameObject* obj, float metallic) { if(obj) obj->material.metallic = metallic; }
    void SetRoughness(GameObject* obj, float roughness) { if(obj) obj->material.roughness = roughness; }
    void SetAO(GameObject* obj, float ao) { if(obj) obj->material.ao = ao; }
    void SetShininess(GameObject* obj, float shininess) { if(obj) obj->material.shininess = shininess; }
    void SetEmissive(GameObject* obj, bool enabled, float intensity, const glm::vec3& color) {
        if(obj) { obj->material.isEmissive = enabled; obj->material.intensity = intensity; obj->material.emissionColor = color; }
    }
    void SetEmissionIntensity(GameObject* obj, float intensity) { if(obj) obj->material.intensity = intensity; }
    void SetTexture(GameObject* obj, const std::string& path) { if(obj) obj->material.diffuseTexture = path; }
    void SetCustomShader(GameObject* obj, const std::string& shaderName) { if(obj) obj->material.customShaderName = shaderName; }
    void SetTransparent(GameObject* obj, bool enabled) { if(obj) obj->material.isTransparent = enabled; }
    void SetBrightness(GameObject* obj, float brightness) { if(obj && obj->hasScreen) obj->screen.brightness = brightness; }
    void SetKeepAspect(GameObject* obj, bool enabled) { if(obj && obj->hasScreen) obj->screen.videoKeepAspect = enabled; }

    void SetSunBloom(float intensity) { Application::Get().GetRenderContext().sunBloom = intensity; }
    void SetMoonBloom(float intensity) { Application::Get().GetRenderContext().moonBloom = intensity; }
    void SetSSR(bool enabled) { Application::Get().GetRenderContext().ssrAll = enabled; }
    void SetSSM(bool enabled) { }
}


namespace Input {
    bool GetKey(int key) { return InputManager::IsKeyPressed(key); }
    bool GetKeyDown(int key) { return InputManager::IsKeyJustPressed(key); }
    bool GetKeyUp(int key) { return !InputManager::IsKeyPressed(key); }
    bool GetMouseButton(int button) { return InputManager::IsMouseButtonPressed(button); }
    glm::vec2 GetMousePos() { return InputManager::GetMousePosition(); }
    float GetMouseX() { return InputManager::GetMouseX(); }
    float GetMouseY() { return InputManager::GetMouseY(); }
    void SetCursorMode(int mode) { InputManager::SetCursorMode(mode); }
    int GetCursorMode() { return InputManager::GetCursorMode(); }
    bool IsButtonClicked(const std::string& name) { return InputManager::IsUIButtonClicked(name); }
}


namespace Scene {
    void Load(const std::string& path) { SceneManager::Get().LoadScene(path); }
    void Save(const std::string& path, bool silent) { if(auto s = SceneManager::Get().GetActiveScene()) s->Save(path, silent); }
    GameObject* Find(const std::string& name) {
        if(auto s = SceneManager::Get().GetActiveScene()) {
            for(auto& obj : s->GetObjects()) if(obj.name == name) return &obj;
        }
        return nullptr;
    }
    GameObject* FindWithTag(const std::string& tag) {
        if(auto s = SceneManager::Get().GetActiveScene()) {
            for(auto& obj : s->GetObjects()) if(obj.tag == tag) return &obj;
        }
        return nullptr;
    }
    std::vector<GameObject*> FindManyWithTag(const std::string& tag) {
        std::vector<GameObject*> result;
        if(auto s = SceneManager::Get().GetActiveScene()) {
            for(auto& obj : s->GetObjects()) if(obj.tag == tag) result.push_back(&obj);
        }
        return result;
    }
    GameObject* GetParent(GameObject* obj) {
        if (!obj || obj->parentIndex == -1) return nullptr;
        return FindByIndex(obj->parentIndex);
    }
    std::vector<GameObject*> GetChildren(GameObject* obj) {
        std::vector<GameObject*> result;
        if (!obj) return result;
        if (auto s = SceneManager::Get().GetActiveScene()) {
            auto& objs = s->GetObjects();
            int myIndex = -1;
            for (int i = 0; i < (int)objs.size(); ++i) if (&objs[i] == obj) { myIndex = i; break; }
            if (myIndex != -1) {
                for (auto& o : objs) if (o.parentIndex == myIndex) result.push_back(&o);
            }
        }
        return result;
    }
    std::vector<GameObject*> GetRootObjects() {
        std::vector<GameObject*> result;
        if (auto s = SceneManager::Get().GetActiveScene()) {
            for (auto& obj : s->GetObjects()) if (obj.parentIndex == -1) result.push_back(&obj);
        }
        return result;
    }
    GameObject* FindByIndex(int index) {
        if(auto s = SceneManager::Get().GetActiveScene()) {
            auto& objs = s->GetObjects();
            if(index >= 0 && index < (int)objs.size()) return &objs[index];
        }
        return nullptr;
    }
    void TransitionTo(const std::string& path, float duration) { SceneManager::Get().TransitionToScene(path, TransitionType::FadeBlack, duration); }
    void JumpToFlag(const std::string& flagName) { SceneManager::Get().JumpToFlag(flagName); }
    void TransitionToFlag(const std::string& flagName, float duration) { SceneManager::Get().TransitionToFlag(flagName, TransitionType::FadeBlack, duration); }
    void Instantiate(const std::string& prefabPath) { }
    void Destroy(GameObject* obj) { if(obj) obj->isActive = false; }
}


namespace Video {
    static void UpdatePlaylistInternal(GameObject* obj) {
        if (!obj || !obj->hasScreen) return;
        auto& s = obj->screen;
        s.videoPlaylist.clear();
        if (s.videoDirectory.empty() || !fs::exists(s.videoDirectory)) return;
        for (const auto& entry : fs::directory_iterator(s.videoDirectory)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                if (ext == ".mp4" || ext == ".mkv" || ext == ".avi" || ext == ".mov") s.videoPlaylist.push_back(entry.path().string());
            }
        }
        std::sort(s.videoPlaylist.begin(), s.videoPlaylist.end());
        if (s.shuffle && !s.videoPlaylist.empty()) {
            static std::random_device rd; static std::mt19937 g(rd());
            std::shuffle(s.videoPlaylist.begin(), s.videoPlaylist.end(), g);
        }
    }

    static void LoadVideoInternal(GameObject* obj, const std::string& path) {
        if (!obj || !obj->hasScreen) return;
        obj->screen.filePath = path;
        if (obj->screen.videoPlayerHandle) {
            VideoPlayer* vp = static_cast<VideoPlayer*>(obj->screen.videoPlayerHandle.get());
            vp->Close();
            if (vp->Open(path)) vp->SetPlaying(!obj->screen.videoPaused);
        }
    }

    void Play(GameObject* obj) { if(obj && obj->hasScreen) { obj->screen.videoPaused = false; if(obj->screen.videoPlayerHandle) static_cast<VideoPlayer*>(obj->screen.videoPlayerHandle.get())->SetPlaying(true); } }
    void Pause(GameObject* obj) { if(obj && obj->hasScreen) { obj->screen.videoPaused = true; if(obj->screen.videoPlayerHandle) static_cast<VideoPlayer*>(obj->screen.videoPlayerHandle.get())->SetPlaying(false); } }
    void Next(GameObject* obj) {
        if (!obj || !obj->hasScreen || obj->screen.videoPlaylist.empty()) return;
        auto& s = obj->screen; s.playlistIndex = (s.playlistIndex + 1) % s.videoPlaylist.size();
        LoadVideoInternal(obj, s.videoPlaylist[s.playlistIndex]);
    }
    void Previous(GameObject* obj) {
        if (!obj || !obj->hasScreen || obj->screen.videoPlaylist.empty()) return;
        auto& s = obj->screen; s.playlistIndex = (s.playlistIndex - 1 + s.videoPlaylist.size()) % s.videoPlaylist.size();
        LoadVideoInternal(obj, s.videoPlaylist[s.playlistIndex]);
    }
    void SetDirectory(GameObject* obj, const std::string& path) {
        if (obj && obj->hasScreen) { obj->screen.videoDirectory = path; obj->screen.playlistMode = !path.empty(); UpdatePlaylistInternal(obj); if (!obj->screen.videoPlaylist.empty()) { obj->screen.playlistIndex = 0; LoadVideoInternal(obj, obj->screen.videoPlaylist[0]); } }
    }
    void RefreshPlaylist(GameObject* obj) { UpdatePlaylistInternal(obj); }
    void SetShuffle(GameObject* obj, bool enabled) { if (obj && obj->hasScreen) { obj->screen.shuffle = enabled; UpdatePlaylistInternal(obj); } }
    void SetLoop(GameObject* obj, bool enabled) { if (obj && obj->hasScreen) obj->screen.videoLoop = enabled; }
    void SetVolume(GameObject* obj, float volume) { if (obj && obj->hasScreen) obj->screen.videoVolume = volume; }
    bool IsPlaying(GameObject* obj) { return obj && obj->hasScreen && !obj->screen.videoPaused; }
    int GetPlaylistCount(GameObject* obj) { return (obj && obj->hasScreen) ? (int)obj->screen.videoPlaylist.size() : 0; }
    int GetPlaylistIndex(GameObject* obj) { return (obj && obj->hasScreen) ? obj->screen.playlistIndex : -1; }
}


namespace Camera {
    void SetFOV(GameObject* obj, float fov) { if(obj && obj->hasCamera) { obj->camera.fov = fov; if(obj->camera.cam) obj->camera.cam->FOV = fov; } }
    float GetFOV(GameObject* obj) { return (obj && obj->hasCamera) ? obj->camera.fov : 0.0f; }
    void SetNearPlane(GameObject* obj, float nearPlane) { if(obj && obj->hasCamera) { obj->camera.nearPlane = nearPlane; if(obj->camera.cam) obj->camera.cam->nearPlane = nearPlane; } }
    void SetFarPlane(GameObject* obj, float farPlane) { if(obj && obj->hasCamera) { obj->camera.farPlane = farPlane; if(obj->camera.cam) obj->camera.cam->farPlane = farPlane; } }
    void SetResolution(GameObject* obj, int width, int height) { if(obj && obj->hasCamera) { obj->camera.resolutionX = width; obj->camera.resolutionY = height; if(obj->camera.cam) obj->camera.cam->UpdateSize(width, height); } }
    void SetEnabled(GameObject* obj, bool enabled) { if(obj && obj->hasCamera) obj->camera.enabled = enabled; }
}


namespace Water {
    void SetWaveSpeed(GameObject* obj, float speed) { if(obj && obj->hasWater) obj->water.waveSpeed = speed; }
    void SetWaveStrength(GameObject* obj, float strength) { if(obj && obj->hasWater) obj->water.waveStrength = strength; }
    void SetColor(GameObject* obj, const glm::vec3& color) { if(obj && obj->hasWater) obj->water.waterColor = color; }
    void SetShininess(GameObject* obj, float shininess) { if(obj && obj->hasWater) obj->water.shininess = shininess; }
    void SetDensity(GameObject* obj, float density) { if(obj && obj->hasWater) obj->water.liquidDensity = density; }
    void SetSurfaceHeight(GameObject* obj, float height) { if(obj && obj->hasWater) obj->water.surfaceHeight = height; }
    void SetDepth(GameObject* obj, float depth) { if(obj && obj->hasWater) obj->water.depth = depth; }
}


namespace Environment {
    static float s_TimeSpeed = 1.0f;
    void SetTimeOfDay(float time) { Application::Get().GetRenderContext().timeOfDay = time; }
    float GetTimeOfDay() { return Application::Get().GetRenderContext().timeOfDay; }
    void SetTimeSpeed(float speed) { s_TimeSpeed = speed; }
    void SetSkyboxEnabled(bool enabled) { Application::Get().GetShowSkybox() = enabled; }
    void SetCloudsEnabled(bool enabled) { Application::Get().GetRenderContext().showClouds = enabled; }
    void SetWaterEnabled(bool enabled) { }
    void SetAmbientColor(const glm::vec3& color) { }
    void SetAmbientIntensity(float intensity) { }
    void SetFogDensity(float density) { }
    void SetFogColor(const glm::vec3& color) { }
    void SetCloudDensity(float density) { Application::Get().GetRenderContext().cloudDensity = density; }
    void SetCloudSpeed(float speed) { }
    void SetCloudHeight(float height) { Application::Get().GetRenderContext().cloudHeight = height; }
}


namespace Optimization {
    void SetStaticBatching(bool enabled) { Renderer::s_StaticBatching = enabled; }
    void SetDynamicBatching(bool enabled) { Renderer::s_DynamicBatching = enabled; }
    void SetAutoLOD(bool enabled) { Renderer::s_AutoLOD = enabled; }
    void SetVRS(bool enabled) { Renderer::s_VRS = enabled; }
    void SetSSR(bool enabled) { }
    void SetHLOD(bool enabled) { Renderer::s_EnableHLOD = enabled; }
    void SetOcclusionCulling(bool enabled) { Renderer::s_EnableOcclusionCulling = enabled; }
}


namespace Light {
    void SetPointLightColor(int index, const glm::vec4& color) {
        if(auto s = SceneManager::Get().GetActiveScene()) {
            auto& pl = s->GetPointLights();
            if(index >= 0 && index < (int)pl.size()) pl[index].color = color;
        }
    }
    void SetPointLightIntensity(int index, float intensity) {
        if(auto s = SceneManager::Get().GetActiveScene()) {
            auto& pl = s->GetPointLights();
            if(index >= 0 && index < (int)pl.size()) pl[index].intensity = intensity;
        }
    }
    void SetPointLightPosition(int index, const glm::vec3& position) {
        if(auto s = SceneManager::Get().GetActiveScene()) {
            auto& pl = s->GetPointLights();
            if(index >= 0 && index < (int)pl.size()) pl[index].position = position;
        }
    }
    void SetPointLightEnabled(int index, bool enabled) {
        if(auto s = SceneManager::Get().GetActiveScene()) {
            auto& pl = s->GetPointLights();
            if(index >= 0 && index < (int)pl.size()) pl[index].enabled = enabled;
        }
    }
    void SetPointLightCastShadows(int index, bool castShadows) {
        if(auto s = SceneManager::Get().GetActiveScene()) {
            auto& pl = s->GetPointLights();
            if(index >= 0 && index < (int)pl.size()) pl[index].castShadows = castShadows;
        }
    }
    int GetPointLightCount() {
        if(auto s = SceneManager::Get().GetActiveScene()) return (int)s->GetPointLights().size();
        return 0;
    }
    int CreatePointLight(const glm::vec3& position, const glm::vec4& color, float intensity) {
        if(auto s = SceneManager::Get().GetActiveScene()) {
            auto pl = s->CreatePointLight();
            if (pl) {
                pl->position = position;
                pl->color = color;
                pl->intensity = intensity;
                return (int)s->GetPointLights().size() - 1;
            }
        }
        return -1;
    }
}


namespace Time {
    static float s_TimeScale = 1.0f;
    static float s_LastDeltaTime = 0.0f;
    static float s_TotalTime = 0.0f;
    float DeltaTime() { return s_LastDeltaTime * s_TimeScale; }
    float TotalTime() { return s_TotalTime; }
    void SetTimeScale(float scale) { s_TimeScale = scale; }
    float GetTimeScale() { return s_TimeScale; }
}

namespace Log {
    void Trace(const std::string& msg) { Logger::AddLog("[Trace] %s", msg.c_str()); }
    void Info(const std::string& msg) { Logger::AddLog("[Info] %s", msg.c_str()); }
    void Warning(const std::string& msg) { Logger::AddLog("[Warning] %s", msg.c_str()); }
    void Error(const std::string& msg) { Logger::AddLog("[Error] %s", msg.c_str()); }
}


namespace SDF {
    void SetEnabled(GameObject* obj, bool enabled) { if(obj) obj->hasSDF = enabled; }
    void SetResolution(GameObject* obj, int resolution) { if(obj) obj->sdf.resolution = resolution; }
    void SetBounds(GameObject* obj, const glm::vec3& minP, const glm::vec3& maxP) { if(obj) { obj->sdf.minP = minP; obj->sdf.maxP = maxP; } }
    void Generate(GameObject* obj) { if(obj) SDFGenerator::GenerateSDF(obj->mesh, obj->sdf.resolution); }
}


namespace Sprite {
    void SetFaceCamera(GameObject* obj, bool faceCamera) { if(obj) obj->sprite.faceCamera = faceCamera; }
    void SetTargetCamera(GameObject* obj, int cameraIndex) { if(obj) obj->sprite.targetCameraIndex = cameraIndex; }
    void SetEnabled(GameObject* obj, bool enabled) { if(obj) obj->is2DSprite = enabled; }
}


namespace UI {
    void CreateButton(const std::string& name, const std::string& text, const glm::vec2& pos, const glm::vec2& size, std::function<void()> onClick) {
        UIElement el; el.name = name; el.text = text; el.position = pos; el.size = size; el.type = UIElementType::BUTTON; el.onClick = onClick;
        UICreationEngine::AddElement(el);
    }
    void CreateText(const std::string& name, const std::string& text, const glm::vec2& pos, const glm::vec4& color) {
        UIElement el; el.name = name; el.text = text; el.position = pos; el.type = UIElementType::TEXT; el.color = color;
        UICreationEngine::AddElement(el);
    }
    void CreateSlider(const std::string& name, const std::string& label, float* value, float min, float max, const glm::vec2& pos, const glm::vec2& size) {
        UIElement el; el.name = name; el.text = label; el.position = pos; el.size = size; el.type = UIElementType::SLIDER;
        UICreationEngine::AddElement(el);
    }
    void CreateCheckbox(const std::string& name, const std::string& label, bool* value, const glm::vec2& pos) {
        UIElement el; el.name = name; el.text = label; el.position = pos; el.type = UIElementType::CHECKBOX;
        UICreationEngine::AddElement(el);
    }
    void CreateImage(const std::string& name, const std::string& texturePath, const glm::vec2& pos, const glm::vec2& size) {
        UIElement el; el.name = name; el.text = texturePath; el.position = pos; el.size = size; el.type = UIElementType::IMAGE;
        UICreationEngine::AddElement(el);
    }
    void SetElementAction(const std::string& name, const std::string& actionType, const std::string& target) {
        for(auto& el : UICreationEngine::GetElements()) {
            if(el.name == name) { el.actionType = actionType; el.targetState = target; break; }
        }
    }
    void RemoveElement(const std::string& name) {
        auto& els = UICreationEngine::GetElements();
        els.erase(std::remove_if(els.begin(), els.end(), [&](const UIElement& e){ return e.name == name; }), els.end());
    }
    void ClearElements() { UICreationEngine::Clear(); }
    void SetScreen(const std::string& screenName) { }

    void SetOnClick(const std::string& name, std::function<void()> callback) { for(auto& el : UICreationEngine::GetElements()) if(el.name == name) { el.onClick = callback; break; } }
    void SetOnHold(const std::string& name, std::function<void()> callback) { for(auto& el : UICreationEngine::GetElements()) if(el.name == name) { el.onHold = callback; break; } }
    void SetOnRelease(const std::string& name, std::function<void()> callback) { for(auto& el : UICreationEngine::GetElements()) if(el.name == name) { el.onRelease = callback; break; } }
    void SetOnHoverEnter(const std::string& name, std::function<void()> callback) { for(auto& el : UICreationEngine::GetElements()) if(el.name == name) { el.onHoverEnter = callback; break; } }
    void SetOnHoverExit(const std::string& name, std::function<void()> callback) { for(auto& el : UICreationEngine::GetElements()) if(el.name == name) { el.onHoverExit = callback; break; } }
}


namespace Math {
    float RandomRange(float min, float max) { static std::mt19937 gen(std::random_device{}()); std::uniform_real_distribution<float> dist(min, max); return dist(gen); }
    int RandomInt(int min, int max) { static std::mt19937 gen(std::random_device{}()); std::uniform_int_distribution<int> dist(min, max); return dist(gen); }
    float Lerp(float a, float b, float t) { return a + t * (b - a); }
    glm::vec3 Lerp(const glm::vec3& a, const glm::vec3& b, float t) { return a + t * (b - a); }
    float Clamp(float val, float min, float max) { return std::max(min, std::min(max, val)); }
    int Clamp(int val, int min, int max) { return std::max(min, std::min(max, val)); }
    float SmoothDamp(float current, float target, float& currentVelocity, float smoothTime, float deltaTime) {
        smoothTime = std::max(0.0001f, smoothTime);
        float omega = 2.0f / smoothTime;
        float x = omega * deltaTime;
        float exp = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);
        float change = current - target;
        float originalTo = target;
        float temp = (currentVelocity + omega * change) * deltaTime;
        currentVelocity = (currentVelocity - omega * temp) * exp;
        float result = target + (change + temp) * exp;
        if (originalTo - current > 0.0f == result > originalTo) { result = originalTo; currentVelocity = (result - originalTo) / deltaTime; }
        return result;
    }
    glm::vec3 SmoothDamp(const glm::vec3& current, const glm::vec3& target, glm::vec3& currentVelocity, float smoothTime, float deltaTime) {
        return glm::vec3(
            SmoothDamp(current.x, target.x, currentVelocity.x, smoothTime, deltaTime),
            SmoothDamp(current.y, target.y, currentVelocity.y, smoothTime, deltaTime),
            SmoothDamp(current.z, target.z, currentVelocity.z, smoothTime, deltaTime)
        );
    }
}

} 
