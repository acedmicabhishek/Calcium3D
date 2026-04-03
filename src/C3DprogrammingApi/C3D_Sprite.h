#ifndef C3D_SPRITE_H
#define C3D_SPRITE_H

struct GameObject;

namespace C3D {
    namespace Sprite {
        void SetFaceCamera(GameObject* obj, bool faceCamera);
        void SetTargetCamera(GameObject* obj, int cameraIndex);
        void SetEnabled(GameObject* obj, bool enabled);
    }
}

#endif
