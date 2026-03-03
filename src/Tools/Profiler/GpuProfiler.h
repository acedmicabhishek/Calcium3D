#pragma once

#ifdef C3D_RUNTIME
    #define GPU_PROFILE_SCOPE(name) ((void)0)
    #define GPU_PROFILE_BEGIN(name) ((void)0)
    #define GPU_PROFILE_END(name)   ((void)0)
#else

#include <string>
#include <vector>
#include <glad/glad.h>

// ─────────────────────────────────────────────────────────────────────────────
// GpuProfiler - OpenGL GL_TIMESTAMP based per-pass GPU timing
//
// How it works:
//   Frame N   : BeginSample() → glQueryCounter(start, GL_TIMESTAMP)
//               EndSample()   → glQueryCounter(end,   GL_TIMESTAMP)
//   Frame N+2 : BeginFrame() reads completed results (1-frame ring buffer lag)
//
// Usage:
//   GPU_PROFILE_SCOPE("GeometryPass");   // RAII, auto end at scope exit
//   GPU_PROFILE_BEGIN("GeometryPass");   // manual
//   GPU_PROFILE_END("GeometryPass");
// ─────────────────────────────────────────────────────────────────────────────


static constexpr int GPU_MAX_SAMPLES   = 48;
static constexpr int GPU_FRAME_LAG     = 5;   
                                               

struct GpuSample {
    std::string name;
    float durationMs = 0.0f;
};

class GpuProfiler {
public:
    static GpuProfiler& Get() {
        static GpuProfiler inst;
        return inst;
    }

    
    void BeginFrame();
    
    void EndFrame();

    void BeginSample(const char* name);
    void EndSample  (const char* name);

    bool IsEnabled() const { return m_Enabled; }
    void SetEnabled(bool v);
    bool IsPaused()  const { return m_Paused; }
    void SetPaused(bool v) { m_Paused = v; }

    const std::vector<GpuSample>& GetLastSamples() const { return m_FinishedSamples; }

private:
    GpuProfiler() = default;

    bool m_Enabled     = false;
    bool m_Paused      = false;
    bool m_Initialized = false;
    int  m_TotalFrames = 0;
    int  m_WriteIdx    = 0;

    void InitQueries();

    struct QuerySlot {
        GLuint qs   = 0;   
        GLuint qe   = 0;   
        char   name[64] = {};
        bool   used = false;
    };

    struct FrameBuffer {
        QuerySlot slots[GPU_MAX_SAMPLES];
        int count = 0;
    };

    FrameBuffer m_Frames[GPU_FRAME_LAG];
    std::vector<GpuSample> m_FinishedSamples;
};


struct GpuProfilerScope {
    const char* m_Name;
    explicit GpuProfilerScope(const char* n) : m_Name(n) { GpuProfiler::Get().BeginSample(n); }
    ~GpuProfilerScope() { GpuProfiler::Get().EndSample(m_Name); }
};

#define GPU_PROFILE_SCOPE(name) GpuProfilerScope _gprof_scope_##__LINE__(name)
#define GPU_PROFILE_BEGIN(name) GpuProfiler::Get().BeginSample(name)
#define GPU_PROFILE_END(name)   GpuProfiler::Get().EndSample(name)

#endif 
