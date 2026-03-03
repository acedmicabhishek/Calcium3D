#include "Scene.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include "../Core/Logger.h"
#include "ObjectFactory.h"
#include "BehaviorRegistry.h"
#include <unordered_map>
#include "../ModelImport/ModelImporter.h"

using json = nlohmann::json;

void Scene::Save(const std::string& path) {
    m_Filepath = path;
    json data;
    
    
    data["physics"]["GlobalGravityEnabled"] = PhysicsEngine::GlobalGravityEnabled;
    data["physics"]["GlobalPhysicsEnabled"] = PhysicsEngine::GlobalPhysicsEnabled;
    data["physics"]["ImpulseEnabled"] = PhysicsEngine::ImpulseEnabled;
    data["physics"]["GlobalCOMEnabled"] = PhysicsEngine::GlobalCOMEnabled;
    data["physics"]["SubSteps"] = PhysicsEngine::SubSteps;
    data["physics"]["LinearDamping"] = PhysicsEngine::LinearDamping;
    data["physics"]["AngularDamping"] = PhysicsEngine::AngularDamping;
    data["physics"]["GlobalAirResistance"] = PhysicsEngine::GlobalAirResistance;
    data["physics"]["Gravity"] = {PhysicsEngine::Gravity.x, PhysicsEngine::Gravity.y, PhysicsEngine::Gravity.z};
    data["physics"]["GlobalAcceleration"] = {PhysicsEngine::GlobalAcceleration.x, PhysicsEngine::GlobalAcceleration.y, PhysicsEngine::GlobalAcceleration.z};
    for (const auto& obj : m_Objects) {
        json jObj;
        jObj["name"] = obj.name;
        jObj["position"] = {obj.position.x, obj.position.y, obj.position.z};
        jObj["rotation"] = {obj.rotation.w, obj.rotation.x, obj.rotation.y, obj.rotation.z};
        jObj["scale"] = {obj.scale.x, obj.scale.y, obj.scale.z};
        
        jObj["useGravity"] = obj.useGravity;
        jObj["isStatic"] = obj.isStatic;
        jObj["mass"] = obj.mass;
        jObj["friction"] = obj.friction;
        jObj["restitution"] = obj.restitution;
        jObj["enableCollision"] = obj.enableCollision;
        jObj["centerOfMassOffset"] = {obj.centerOfMassOffset.x, obj.centerOfMassOffset.y, obj.centerOfMassOffset.z};
        jObj["angularVelocity"] = {obj.angularVelocity.x, obj.angularVelocity.y, obj.angularVelocity.z};
        jObj["shape"] = static_cast<int>(obj.shape);
        jObj["collisionRadius"] = obj.collisionRadius;
        jObj["scripts"] = obj.scriptNames;
        jObj["material"] = obj.material.Serialize();
        jObj["modelPath"] = obj.modelPath;
        jObj["meshIndex"] = obj.meshIndex;
        jObj["meshType"] = static_cast<int>(obj.meshType);
        jObj["parentIndex"] = obj.parentIndex;
        jObj["isFolded"] = obj.isFolded;
        jObj["isActive"] = obj.isActive;
        
        jObj["acousticMaterial"] = {
            {"hardness", obj.acousticMaterial.hardness},
            {"absorption", obj.acousticMaterial.absorption},
            {"isAcousticObstacle", obj.acousticMaterial.isAcousticObstacle}
        };
        
        jObj["hasAudio"] = obj.hasAudio;
        jObj["audio"] = {
            {"filePath", obj.audio.filePath},
            {"volume", obj.audio.volume},
            {"pitch", obj.audio.pitch},
            {"looping", obj.audio.looping},
            {"minDistance", obj.audio.minDistance},
            {"maxDistance", obj.audio.maxDistance},
            {"type", static_cast<int>(obj.audio.type)},
            {"playOnAwake", obj.audio.playOnAwake},
            {"enableDoppler", obj.audio.enableDoppler},
            {"dopplerFactor", obj.audio.dopplerFactor},
            {"enableReverb", obj.audio.enableReverb},
            {"enableOcclusion", obj.audio.enableOcclusion}
        };
        
        data["objects"].push_back(jObj);
    }

    
    for (const auto& pl : m_PointLights) {
        json jPl;
        jPl["position"] = {pl.position.x, pl.position.y, pl.position.z};
        jPl["color"] = {pl.color.r, pl.color.g, pl.color.b, pl.color.a};
        jPl["intensity"] = pl.intensity;
        jPl["enabled"] = pl.enabled;
        data["point_lights"].push_back(jPl);
    }

    std::ofstream file(path);
    if (file.is_open()) {
        file << data.dump(4);
        m_Filepath = path;
        Logger::AddLog("Scene saved to %s", path.c_str());
    }
}

void Scene::Load(const std::string& path) {
    m_Filepath = path;
    std::ifstream file(path);
    if (!file.is_open()) {
        Logger::AddLog("[ERROR] Failed to open scene file: %s", path.c_str());
        return;
    }

    try {
        json data = json::parse(file);
        Clear();

        
        if (data.contains("physics")) {
            auto& phys = data["physics"];
            if (phys.contains("GlobalGravityEnabled")) PhysicsEngine::GlobalGravityEnabled = phys["GlobalGravityEnabled"];
            if (phys.contains("GlobalPhysicsEnabled")) PhysicsEngine::GlobalPhysicsEnabled = phys["GlobalPhysicsEnabled"];
            if (phys.contains("ImpulseEnabled")) PhysicsEngine::ImpulseEnabled = phys["ImpulseEnabled"];
            if (phys.contains("GlobalCOMEnabled")) PhysicsEngine::GlobalCOMEnabled = phys["GlobalCOMEnabled"];
            if (phys.contains("SubSteps")) PhysicsEngine::SubSteps = phys["SubSteps"];
            if (phys.contains("LinearDamping")) PhysicsEngine::LinearDamping = phys["LinearDamping"];
            if (phys.contains("AngularDamping")) PhysicsEngine::AngularDamping = phys["AngularDamping"];
            if (phys.contains("GlobalAirResistance")) PhysicsEngine::GlobalAirResistance = phys["GlobalAirResistance"];
            
            if (phys.contains("Gravity")) {
                auto g = phys["Gravity"];
                PhysicsEngine::Gravity = glm::vec3(g[0], g[1], g[2]);
            }
            if (phys.contains("GlobalAcceleration")) {
                auto ga = phys["GlobalAcceleration"];
                PhysicsEngine::GlobalAcceleration = glm::vec3(ga[0], ga[1], ga[2]);
            }
        }

        if (data.contains("objects")) {
            std::unordered_map<std::string, ImportResult> importCache;
            
            for (const auto& jObj : data["objects"]) {
                ColliderShape loadedShape = ColliderShape::Box;
                if (jObj.contains("shape")) {
                    loadedShape = static_cast<ColliderShape>(jObj["shape"].get<int>());
                }
                
                std::string modelPath = "";
                if (jObj.contains("modelPath")) modelPath = jObj["modelPath"].get<std::string>();
                int meshIndex = -1;
                if (jObj.contains("meshIndex")) meshIndex = jObj["meshIndex"].get<int>();
                
                MeshType loadedType = MeshType::None;
                if (jObj.contains("meshType")) loadedType = static_cast<MeshType>(jObj["meshType"].get<int>());

                GameObject* objPtr = nullptr;
                
                if (loadedType == MeshType::Model && !modelPath.empty() && meshIndex != -1) {
                    if (importCache.find(modelPath) == importCache.end()) {
                        importCache[modelPath] = ModelImporter::Import(modelPath);
                        if (!importCache[modelPath].success) {
                            Logger::AddLog("[SceneIO] [WARNING] Failed to re-import model: %s. Error: %s", 
                                          modelPath.c_str(), importCache[modelPath].error.c_str());
                        }
                    }
                    
                    auto& result = importCache[modelPath];
                    if (result.success && meshIndex < (int)result.meshes.size()) {
                        auto& meshData = result.meshes[meshIndex];
                        Mesh mesh(meshData.vertices, meshData.indices, meshData.textures);
                        objPtr = new GameObject(std::move(mesh), jObj["name"]);
                        objPtr->modelPath = modelPath;
                        objPtr->meshIndex = meshIndex;
                        objPtr->meshType = MeshType::Model;
                        objPtr->isStatic = true; 
                    }
                }

                if (!objPtr) {
                    if (loadedType == MeshType::Cube) {
                        objPtr = new GameObject(ObjectFactory::createCube(), jObj["name"]);
                    } else if (loadedType == MeshType::Sphere) {
                        objPtr = new GameObject(ObjectFactory::createSphere(30,30), jObj["name"]);
                    } else if (loadedType == MeshType::Plane) {
                        objPtr = new GameObject(ObjectFactory::createPlane(), jObj["name"]);
                    } else {
                        
                        std::vector<Vertex> rawVerts;
                        std::vector<GLuint> rawIndices;
                        objPtr = new GameObject(Mesh(rawVerts, rawIndices, {}), jObj["name"]);
                    }
                    objPtr->meshType = loadedType;
                }
                
                GameObject& obj = *objPtr;
                obj.shape = loadedShape;
                
                auto pos = jObj["position"];
                obj.position = glm::vec3(pos[0], pos[1], pos[2]);
                
                auto rot = jObj["rotation"];
                obj.rotation = glm::quat(rot[0], rot[1], rot[2], rot[3]);
                
                auto sc = jObj["scale"];
                obj.scale = glm::vec3(sc[0], sc[1], sc[2]);
                
                if (jObj.contains("useGravity")) obj.useGravity = jObj["useGravity"];
                if (jObj.contains("isStatic")) obj.isStatic = jObj["isStatic"];
                if (jObj.contains("mass")) obj.mass = jObj["mass"];
                if (jObj.contains("friction")) obj.friction = jObj["friction"];
                if (jObj.contains("restitution")) obj.restitution = jObj["restitution"];
                if (jObj.contains("enableCollision")) obj.enableCollision = jObj["enableCollision"];
                if (jObj.contains("centerOfMassOffset")) {
                    auto com = jObj["centerOfMassOffset"];
                    obj.centerOfMassOffset = glm::vec3(com[0], com[1], com[2]);
                }
                
                if (jObj.contains("angularVelocity")) {
                    auto av = jObj["angularVelocity"];
                    obj.angularVelocity = glm::vec3(av[0], av[1], av[2]);
                }
                if (jObj.contains("collisionRadius")) {
                    obj.collisionRadius = jObj["collisionRadius"];
                }

                if (jObj.contains("scripts")) {
                    for (const auto& sName : jObj["scripts"]) {
                        auto behavior = BehaviorRegistry::Create(sName);
                        if (behavior) {
                            behavior->gameObject = nullptr; 
                            obj.behaviors.push_back(std::move(behavior));
                            obj.scriptNames.push_back(sName);
                        }
                    }
                }

                if (jObj.contains("material")) {
                    obj.material.Deserialize(jObj["material"]);
                }
                
                if (jObj.contains("parentIndex")) obj.parentIndex = jObj["parentIndex"];
                if (jObj.contains("isFolded")) obj.isFolded = jObj["isFolded"];
                if (jObj.contains("isActive")) obj.isActive = jObj["isActive"];
                
                if (jObj.contains("acousticMaterial")) {
                    auto& am = jObj["acousticMaterial"];
                    if (am.contains("hardness")) obj.acousticMaterial.hardness = am["hardness"];
                    if (am.contains("absorption")) obj.acousticMaterial.absorption = am["absorption"];
                    if (am.contains("isAcousticObstacle")) obj.acousticMaterial.isAcousticObstacle = am["isAcousticObstacle"];
                }
                
                if (jObj.contains("hasAudio")) obj.hasAudio = jObj["hasAudio"];
                if (jObj.contains("audio")) {
                    auto& a = jObj["audio"];
                    if (a.contains("filePath")) obj.audio.filePath = a["filePath"];
                    if (a.contains("volume")) obj.audio.volume = a["volume"];
                    if (a.contains("pitch")) obj.audio.pitch = a["pitch"];
                    if (a.contains("looping")) obj.audio.looping = a["looping"];
                    if (a.contains("minDistance")) obj.audio.minDistance = a["minDistance"];
                    if (a.contains("maxDistance")) obj.audio.maxDistance = a["maxDistance"];
                    if (a.contains("type")) obj.audio.type = static_cast<AudioType>(a["type"].get<int>());
                    if (a.contains("playOnAwake")) obj.audio.playOnAwake = a["playOnAwake"];
                    
                    if (a.contains("enableDoppler")) obj.audio.enableDoppler = a["enableDoppler"];
                    if (a.contains("dopplerFactor")) obj.audio.dopplerFactor = a["dopplerFactor"];
                    if (a.contains("enableReverb")) obj.audio.enableReverb = a["enableReverb"];
                    if (a.contains("enableOcclusion")) obj.audio.enableOcclusion = a["enableOcclusion"];
                }
                
                AddObject(std::move(obj));
                delete objPtr;
                
                auto& addedObj = m_Objects.back();
                for (auto& script : addedObj.behaviors) {
                    script->gameObject = &addedObj;
                    script->OnStart();
                }
            }
        }

        if (data.contains("point_lights")) {
            for (const auto& jPl : data["point_lights"]) {
                PointLight* pl = CreatePointLight();
                
                auto pos = jPl["position"];
                pl->position = glm::vec3(pos[0], pos[1], pos[2]);
                
                auto col = jPl["color"];
                pl->color = glm::vec4(col[0], col[1], col[2], col[3]);
                
                pl->intensity = jPl["intensity"];
                pl->enabled = jPl["enabled"];
            }
        }

        Logger::AddLog("Scene loaded from %s", path.c_str());
    } catch (const std::exception& e) {
        Logger::AddLog("[ERROR] Scene load failed: %s", e.what());
    }
}
