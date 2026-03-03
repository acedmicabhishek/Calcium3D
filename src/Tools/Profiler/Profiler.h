#pragma once



#ifdef C3D_RUNTIME
    #define PROFILE_SCOPE(name)  ((void)0)
    #define PROFILE_BEGIN(name)  ((void)0)
    #define PROFILE_END(name)    ((void)0)
#else

#include <string>
#include <vector>
#include <chrono>
#include <unordered_map>
#include <array>

#define PROFILE_SCOPE(name) ProfilerScope _prof_scope_##__LINE__(name)
#define PROFILE_BEGIN(name) Profiler::Get().BeginSample(name)
#define PROFILE_END(name)   Profiler::Get().EndSample(name)

static constexpr int PROFILER_HISTORY = 256;

struct ProfileSample {
    std::string name;
    float durationMs = 0.0f;
};

struct ProfileFrame {
    float totalMs = 0.0f;
    float fps = 0.0f;
    std::vector<ProfileSample> samples;
};

class Profiler {
public:
    static Profiler& Get() {
        static Profiler instance;
        return instance;
    }

    void BeginSample(const std::string& name);
    void EndSample(const std::string& name);
    void BeginFrame();
    void EndFrame(float deltaTimeMs = -1.0f); 

    bool IsEnabled() const { return m_Enabled; }
    void SetEnabled(bool v) { m_Enabled = v; }
    bool IsPaused()  const { return m_Paused; }
    void SetPaused(bool v) { m_Paused = v; }

    
    const std::array<ProfileFrame, PROFILER_HISTORY>& GetFrameHistory() const { return m_FrameHistory; }
    int GetCurrentFrameIdx() const { return m_FrameIdx; }
    const ProfileFrame& GetLastFrame() const { return m_FrameHistory[(m_FrameIdx - 1 + PROFILER_HISTORY) % PROFILER_HISTORY]; }
    float GetAvgFPS() const { return m_AvgFps; }

    
    static const char* GetCategoryColor(const std::string& name);

private:
    Profiler() = default;
    bool m_Enabled = false;
    bool m_Paused  = false;

    struct ActiveSample {
        std::chrono::high_resolution_clock::time_point start;
    };

    std::unordered_map<std::string, ActiveSample> m_Active;
    std::vector<ProfileSample>                    m_CurrentSamples;
    std::chrono::high_resolution_clock::time_point m_FrameStart;
    float m_FrameStartMs = 0.0f;

    std::array<ProfileFrame, PROFILER_HISTORY> m_FrameHistory{};
    int   m_FrameIdx = 0;
    float m_AvgFps   = 0.0f;
};


struct ProfilerScope {
    explicit ProfilerScope(const char* name) : m_Name(name) {
        Profiler::Get().BeginSample(m_Name);
    }
    ~ProfilerScope() { Profiler::Get().EndSample(m_Name); }
private:
    const char* m_Name;
};

#endif 
