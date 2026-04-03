#ifndef C3D_AUDIO_H
#define C3D_AUDIO_H

#include <string>

struct GameObject;

namespace C3D {
    namespace Audio {
        void Play(GameObject* obj);
        void Stop(GameObject* obj);
        void SetClip(GameObject* obj, const std::string& path);
        
        void SetVolume(GameObject* obj, float volume);
        void SetPitch(GameObject* obj, float pitch);
        void SetLooping(GameObject* obj, bool loop);
        
        void SetSpatial(GameObject* obj, bool spatial);
        void SetDistances(GameObject* obj, float minDistance, float maxDistance);
        
        
        void SetDopplerFactor(GameObject* obj, float factor);
        void SetReverbEnabled(GameObject* obj, bool enabled);
        void SetOcclusionEnabled(GameObject* obj, bool enabled);
    }
}

#endif
