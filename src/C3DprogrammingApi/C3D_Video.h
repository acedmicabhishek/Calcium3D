#ifndef C3D_VIDEO_H
#define C3D_VIDEO_H

#include <string>

struct GameObject;

namespace C3D {
    namespace Video {
        void Play(GameObject* obj);
        void Pause(GameObject* obj);
        void Next(GameObject* obj);
        void Previous(GameObject* obj);
        
        void SetDirectory(GameObject* obj, const std::string& path);
        void RefreshPlaylist(GameObject* obj);
        
        void SetShuffle(GameObject* obj, bool enabled);
        void SetLoop(GameObject* obj, bool enabled);
        void SetVolume(GameObject* obj, float volume);
        
        bool IsPlaying(GameObject* obj);
        int GetPlaylistCount(GameObject* obj);
        int GetPlaylistIndex(GameObject* obj);
    }
}

#endif
