#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <string>

class VideoPlayer {
public:
    VideoPlayer();
    ~VideoPlayer();

    bool Open(const std::string& path);
    void Close();
    void Update(float dt);
    
    unsigned int GetTextureID() const { return m_TextureID; }
    bool IsPlaying() const { return m_Playing; }
    void SetPlaying(bool playing) { m_Playing = playing; }
    
    void SetLooping(bool looping) { m_Looping = looping; }
    int GetWidth() const;
    int GetHeight() const;

private:
    unsigned int m_TextureID;
    bool m_Playing;
    bool m_Looping;
    
    struct FFmpegContext;
    FFmpegContext* m_Ctx;
};

#endif
