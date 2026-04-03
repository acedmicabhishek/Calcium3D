#ifndef C3D_SCENE_H
#define C3D_SCENE_H

#include <string>
#include <vector>

struct GameObject;

namespace C3D {
    namespace Scene {
        void Load(const std::string& path);
        void Save(const std::string& path, bool silent = true);
        
        GameObject* Find(const std::string& name);
        GameObject* FindWithTag(const std::string& tag);
        std::vector<GameObject*> FindManyWithTag(const std::string& tag);
        GameObject* FindByIndex(int index);
        
        void TransitionTo(const std::string& path, float duration = 1.0f);
        void JumpToFlag(const std::string& flagName);
        void TransitionToFlag(const std::string& flagName, float duration = 1.0f);
        
        void Instantiate(const std::string& prefabPath);
        void Destroy(GameObject* obj);
        
        GameObject* GetParent(GameObject* obj);
        std::vector<GameObject*> GetChildren(GameObject* obj);
        std::vector<GameObject*> GetRootObjects();
    }
}

#endif
