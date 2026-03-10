#ifndef C3D_RUNTIME
#include "Profiler.h"
#include <algorithm>

void Profiler::BeginFrame() {
  if (!m_Enabled || m_Paused)
    return;
  m_FrameStart = std::chrono::high_resolution_clock::now();
  m_CurrentSamples.clear();
}

void Profiler::EndFrame(float deltaTimeMs) {
  if (!m_Enabled || m_Paused)
    return;
  auto now = std::chrono::high_resolution_clock::now();

  float totalMs =
      (deltaTimeMs > 0.0f)
          ? deltaTimeMs
          : std::chrono::duration<float, std::milli>(now - m_FrameStart)
                .count();

  ProfileFrame frame;
  frame.totalMs = totalMs;
  frame.fps = (totalMs > 0.0f) ? (1000.0f / totalMs) : 0.0f;
  frame.samples = m_CurrentSamples;

  m_FrameHistory[m_FrameIdx] = std::move(frame);
  m_FrameIdx = (m_FrameIdx + 1) % PROFILER_HISTORY;

  float sum = 0.0f;
  int count = 0;
  for (auto &f : m_FrameHistory) {
    if (f.fps > 0.0f) {
      sum += f.fps;
      count++;
    }
  }
  m_AvgFps = (count > 0) ? (sum / (float)count) : 0.0f;
}

void Profiler::BeginSample(const std::string &name) {
  if (!m_Enabled || m_Paused)
    return;
  m_Active[name] = {std::chrono::high_resolution_clock::now()};
}

void Profiler::EndSample(const std::string &name) {
  if (!m_Enabled || m_Paused)
    return;
  auto it = m_Active.find(name);
  if (it == m_Active.end())
    return;

  auto now = std::chrono::high_resolution_clock::now();
  float ms =
      std::chrono::duration<float, std::milli>(now - it->second.start).count();

  bool found = false;
  for (auto &s : m_CurrentSamples) {
    if (s.name == name) {
      s.durationMs += ms;
      found = true;
      break;
    }
  }
  if (!found) {
    m_CurrentSamples.push_back({name, ms});
  }

  m_Active.erase(it);
}

const char *Profiler::GetCategoryColor(const std::string &name) {

  if (name == "Render" || name == "GeometryPass" || name == "ShadowPass" ||
      name == "SkyPass" || name == "TransparencyPass")
    return "render";
  if (name == "Physics")
    return "physics";
  if (name == "Scripts")
    return "scripts";
  if (name == "UI")
    return "ui";
  if (name == "Audio")
    return "audio";
  return "other";
}
#endif
