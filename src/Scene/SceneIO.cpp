#include "Scene.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include "../Core/Logger.h"
#include "ObjectFactory.h"

using json = nlohmann::json;

void Scene::Save(const std::string& path) {
    json data;
    
    
    for (const auto& obj : m_Objects) {
        json jObj;
        jObj["name"] = obj.name;
        jObj["position"] = {obj.position.x, obj.position.y, obj.position.z};
        jObj["rotation"] = {obj.rotation.w, obj.rotation.x, obj.rotation.y, obj.rotation.z};
        jObj["scale"] = {obj.scale.x, obj.scale.y, obj.scale.z};
        
        jObj["useGravity"] = obj.useGravity;
        jObj["isStatic"] = obj.isStatic;
        
        
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
        Logger::AddLog("Scene saved to %s", path.c_str());
    }
}

void Scene::Load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        Logger::AddLog("[ERROR] Failed to open scene file: %s", path.c_str());
        return;
    }

    try {
        json data = json::parse(file);
        Clear();

        if (data.contains("objects")) {
            for (const auto& jObj : data["objects"]) {
                
                Mesh mesh = ObjectFactory::createCube();
                GameObject obj(std::move(mesh), jObj["name"]);
                
                auto pos = jObj["position"];
                obj.position = glm::vec3(pos[0], pos[1], pos[2]);
                
                auto rot = jObj["rotation"];
                obj.rotation = glm::quat(rot[0], rot[1], rot[2], rot[3]);
                
                auto sc = jObj["scale"];
                obj.scale = glm::vec3(sc[0], sc[1], sc[2]);
                
                if (jObj.contains("useGravity")) obj.useGravity = jObj["useGravity"];
                if (jObj.contains("isStatic")) obj.isStatic = jObj["isStatic"];
                
                AddObject(std::move(obj));
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
