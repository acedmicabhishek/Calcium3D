#ifndef C3D_RUNTIME
#include "GpuProfiler.h"
#include <algorithm>
#include <cstring>

void GpuProfiler::SetEnabled(bool v) {
  if (v && !m_Initialized)
    InitQueries();
  m_Enabled = v;
  if (!v)
    m_FinishedSamples.clear();
}

void GpuProfiler::InitQueries() {
  for (int f = 0; f < GPU_FRAME_LAG; f++) {
    for (int i = 0; i < GPU_MAX_SAMPLES; i++) {
      glGenQueries(1, &m_Frames[f].slots[i].qs);
      glGenQueries(1, &m_Frames[f].slots[i].qe);
    }
  }
  m_Initialized = true;
}

void GpuProfiler::BeginFrame() {
  if (!m_Enabled || m_Paused)
    return;
  if (!m_Initialized)
    InitQueries();

  if (m_TotalFrames >= GPU_FRAME_LAG) {
    int readIdx = (m_WriteIdx + 1) % GPU_FRAME_LAG;
    FrameBuffer &readFrame = m_Frames[readIdx];

    std::vector<GpuSample> newSamples;
    bool allReady = true;
    for (int i = 0; i < readFrame.count; i++) {
      auto &slot = readFrame.slots[i];
      if (!slot.used)
        continue;

      GLint avail = 0;
      glGetQueryObjectiv(slot.qs, GL_QUERY_RESULT_AVAILABLE, &avail);
      if (!avail) {
        allReady = false;
        continue;
      }

      GLuint64 t0 = 0, t1 = 0;
      glGetQueryObjectui64v(slot.qs, GL_QUERY_RESULT, &t0);
      glGetQueryObjectui64v(slot.qe, GL_QUERY_RESULT, &t1);
      float ms = (t1 >= t0) ? (float)(t1 - t0) / 1e6f : 0.0f;

      bool found = false;
      for (auto &s : newSamples) {
        if (s.name == slot.name) {
          s.durationMs += ms;
          found = true;
          break;
        }
      }
      if (!found) {
        newSamples.push_back({slot.name, ms});
      }
    }

    if (!newSamples.empty())
      m_FinishedSamples = newSamples;

    (void)allReady;
  }

  FrameBuffer &writeFrame = m_Frames[m_WriteIdx];
  for (int i = 0; i < writeFrame.count; i++)
    writeFrame.slots[i].used = false;
  writeFrame.count = 0;
}

void GpuProfiler::EndFrame() {
  if (!m_Enabled || m_Paused)
    return;
  m_TotalFrames++;
  m_WriteIdx = (m_WriteIdx + 1) % GPU_FRAME_LAG;
}

void GpuProfiler::BeginSample(const char *name) {
  if (!m_Enabled || m_Paused || !m_Initialized)
    return;
  FrameBuffer &f = m_Frames[m_WriteIdx];
  if (f.count >= GPU_MAX_SAMPLES)
    return;

  auto &slot = f.slots[f.count];
  strncpy(slot.name, name, sizeof(slot.name) - 1);
  slot.name[sizeof(slot.name) - 1] = '\0';
  slot.used = true;
  f.count++;

  glQueryCounter(slot.qs, GL_TIMESTAMP);
}

void GpuProfiler::EndSample(const char *name) {
  if (!m_Enabled || m_Paused || !m_Initialized)
    return;
  FrameBuffer &f = m_Frames[m_WriteIdx];

  for (int i = f.count - 1; i >= 0; i--) {
    if (f.slots[i].used &&
        strncmp(f.slots[i].name, name, sizeof(f.slots[i].name)) == 0) {
      glQueryCounter(f.slots[i].qe, GL_TIMESTAMP);
      return;
    }
  }
}
#endif
