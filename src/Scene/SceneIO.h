#ifndef SCENE_IO_H
#define SCENE_IO_H

#include "Scene.h"
#include <nlohmann/json.hpp>

class SceneIO {
public:
    static nlohmann::json SerializeObject(const GameObject& obj);
    static void DeserializeObject(const nlohmann::json& data, GameObject& obj);
    
    
    static void SavePrefab(const GameObject& obj, const std::string& path);
    static GameObject LoadPrefab(const std::string& path);
};

#endif
