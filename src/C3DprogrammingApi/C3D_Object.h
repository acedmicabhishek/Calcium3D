#ifndef C3D_OBJECT_H
#define C3D_OBJECT_H

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct GameObject;

namespace C3D {
    namespace Object {
        void SetPosition(GameObject* obj, const glm::vec3& pos);
        glm::vec3 GetPosition(GameObject* obj);
        
        void SetRotation(GameObject* obj, const glm::quat& rot);
        void SetRotationEuler(GameObject* obj, const glm::vec3& euler);
        glm::quat GetRotation(GameObject* obj);
        
        void SetScale(GameObject* obj, const glm::vec3& scale);
        glm::vec3 GetScale(GameObject* obj);
        
        void SetActive(GameObject* obj, bool active);
        bool IsActive(GameObject* obj);
        
        const std::string& GetName(GameObject* obj);
        void SetName(GameObject* obj, const std::string& name);
        const std::string& GetTag(GameObject* obj);
        void SetTag(GameObject* obj, const std::string& tag);
        
        void SetParent(GameObject* obj, int parentIndex);
        int GetParentIndex(GameObject* obj);
        
        int GetCount();
        void Duplicate(int index);
        void Destroy(int index);
    }
}

#endif
