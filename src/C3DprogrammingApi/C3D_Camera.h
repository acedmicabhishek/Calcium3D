#ifndef C3D_CAMERA_H
#define C3D_CAMERA_H

struct GameObject;

namespace C3D {
    namespace Camera {
        void SetFOV(GameObject* obj, float fov);
        float GetFOV(GameObject* obj);
        
        void SetNearPlane(GameObject* obj, float nearPlane);
        void SetFarPlane(GameObject* obj, float farPlane);
        
        void SetResolution(GameObject* obj, int width, int height);
        void SetEnabled(GameObject* obj, bool enabled);
    }
}

#endif
