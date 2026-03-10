#ifndef C3D_RUNTIME
#include "ProfilerUI.h"
#include "GpuProfiler.h"
#include "Profiler.h"
#include <algorithm>
#include <imgui.h>
#include <string>
#include <vector>

struct CatDef {
  const char *label;
  ImVec4 color;
};
static const CatDef CATS[] = {
    { "GeometryPass",     {0.18f,0.60f,0.95f,1.0f} },
    { "SkyPass",          {0.40f,0.80f,0.40f,1.0f} },
    { "TransparencyPass", {0.30f,0.55f,0.85f,1.0f} },
    { "RenderPipeline",   {0.20f,0.70f,0.80f,1.0f} },
    { "Physics",          {0.95f,0.40f,0.20f,1.0f} },
    { "Scripts",          {0.95f,0.70f,0.20f,1.0f} },
    { "Camera",           {0.70f,0.35f,0.90f,1.0f} },
    { "UI",               {0.85f,0.75f,0.20f,1.0f} },
    { "Scripts+Physics",  {0.95f,0.55f,0.20f,1.0f} },
};
static constexpr int NUM_CATS = (int)(sizeof(CATS) / sizeof(CATS[0]));

static ImVec4 GetColor(const std::string &name) {
  for (auto &c : CATS)
    if (name == c.label)
      return c.color;
  return {0.55f, 0.55f, 0.55f, 1.0f};
}
static ImU32 GetU32(const std::string &name) {
  return ImGui::ColorConvertFloat4ToU32(GetColor(name));
}

static int s_SelFrame = -1;

void ProfilerUI::Draw(bool* pOpen) {
    auto& prof = Profiler::Get();
    auto& gpu  = GpuProfiler::Get();
    const auto& history = prof.GetFrameHistory();
    int   curIdx = prof.GetCurrentFrameIdx();
  float avgFps = prof.GetAvgFPS();

  ImGui::SetNextWindowSize(ImVec2(880, 560), ImGuiCond_FirstUseEver);
  ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.08f, 0.10f, 0.97f));
  ImGui::PushStyleColor(ImGuiCol_TitleBgActive,
                        ImVec4(0.12f, 0.12f, 0.16f, 1.0f));
  if (!ImGui::Begin("Profiler##C3D", pOpen)) {
    ImGui::PopStyleColor(2);
    ImGui::End();
    return;
  }
  ImGui::PopStyleColor(2);

  bool enabled = prof.IsEnabled();

  ImGui::PushStyleColor(ImGuiCol_Button, enabled
                                             ? ImVec4(0.1f, 0.7f, 0.3f, 1.0f)
                                             : ImVec4(0.3f, 0.3f, 0.35f, 1.0f));
  if (ImGui::Button(enabled ? "  ● REC  " : "  ○ REC  "))
    prof.SetEnabled(!enabled);
  ImGui::PopStyleColor();
  ImGui::SameLine();
  bool paused = prof.IsPaused();
  if (ImGui::Button(paused ? "  ▶ Resume  " : "  ⏸ Pause  "))
    prof.SetPaused(!paused);
  ImGui::SameLine();
  if (ImGui::Button("  ✖ Clear  ")) {
    for (auto &f :
         const_cast<std::array<ProfileFrame, PROFILER_HISTORY> &>(history))
      f = {};
    s_SelFrame = -1;
  }

  const ProfileFrame &liveFrame =
      (s_SelFrame >= 0) ? history[s_SelFrame] : prof.GetLastFrame();
  float liveFps = liveFrame.fps, liveMs = liveFrame.totalMs;

  ImGui::SameLine(0, 16);
  ImVec4 fpsCol = liveFps >= 55.f   ? ImVec4(0.3f, 0.9f, 0.4f, 1)
                  : liveFps >= 30.f ? ImVec4(0.9f, 0.75f, 0.2f, 1)
                                    : ImVec4(0.9f, 0.25f, 0.25f, 1);
  ImGui::PushStyleColor(ImGuiCol_Text, fpsCol);
  ImGui::Text("%.1f FPS  (%.2f ms)", liveFps, liveMs);
  ImGui::PopStyleColor();
  ImGui::SameLine(0, 12);
  ImGui::TextDisabled("Avg %.0f FPS", avgFps);

  ImGui::SameLine(0, 16);
  bool gpuHasData = !gpu.GetLastSamples().empty();
  ImGui::PushStyleColor(ImGuiCol_Text, gpuHasData
                                           ? ImVec4(0.3f, 0.9f, 0.4f, 1)
                                           : ImVec4(0.6f, 0.4f, 0.2f, 1));
  ImGui::Text(gpuHasData ? " GPU Queries Active"
                         : "  GPU Queries (waiting...)");
  ImGui::PopStyleColor();
  ImGui::Separator();

  float maxFrameMs = 20.0f;
  for (int i = 0; i < PROFILER_HISTORY; i++)
    if (history[i].totalMs > maxFrameMs)
      maxFrameMs = history[i].totalMs;
  float maxMs = std::max(maxFrameMs * 1.2f, 16.7f);

  float sideW = 150.0f;
  float innerH = ImGui::GetContentRegionAvail().y;
  float graphH = 95.0f;
  float tableH = innerH - graphH - 90.0f;

  ImGui::BeginChild("##sidebar", ImVec2(sideW, innerH), false);
  ImGui::TextDisabled("CPU & GPU Usage");
  ImGui::Spacing();
  for (auto &c : CATS) {
    ImGui::PushStyleColor(ImGuiCol_Text, c.color);
    ImGui::Bullet();
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::TextUnformatted(c.label);
  }
  ImGui::Spacing();
  ImGui::TextDisabled("Render Passes");
  ImGui::TextDisabled("  Geometry");
  ImGui::TextDisabled("  Sky");
  ImGui::TextDisabled("  Transparency");
  ImGui::EndChild();
  ImGui::SameLine();

  ImGui::BeginChild("##right", ImVec2(0, innerH), false);
  ImDrawList *draw = ImGui::GetWindowDrawList();

  ImVec2 gPos = ImGui::GetCursorScreenPos();
  float gW = ImGui::GetContentRegionAvail().x;
  draw->AddRectFilled(gPos, ImVec2(gPos.x + gW, gPos.y + graphH),
                      IM_COL32(15, 15, 20, 255), 4.0f);

  auto yForMs = [&](float t) { return gPos.y + graphH - (t / maxMs) * graphH; };
  for (float tgt : {16.7f, 33.3f}) {
    if (tgt >= maxMs)
      continue;
    float y = yForMs(tgt);
    draw->AddLine(ImVec2(gPos.x, y), ImVec2(gPos.x + gW, y),
                  IM_COL32(200, 200, 80, 50));
    char lb[12];
    snprintf(lb, sizeof(lb), "%.0fms", tgt);
    draw->AddText(ImVec2(gPos.x + 4, y - 14), IM_COL32(200, 200, 80, 130), lb);
  }

  float barW = gW / (float)PROFILER_HISTORY;
  for (int i = 0; i < PROFILER_HISTORY; i++) {
    int idx = (curIdx + i) % PROFILER_HISTORY;
    const ProfileFrame &f = history[idx];
    if (f.totalMs <= 0.f)
      continue;
    float x0 = gPos.x + i * barW;
    float x1 = x0 + std::max(barW - 1.f, 1.f);
    bool sel = (s_SelFrame == idx);

    float totalH = std::min((f.totalMs / maxMs) * graphH, graphH);
    draw->AddRectFilled(
        ImVec2(x0, gPos.y + graphH - totalH), ImVec2(x1, gPos.y + graphH),
        sel ? IM_COL32(90, 90, 110, 200) : IM_COL32(50, 50, 60, 200));

    float yBase = gPos.y + graphH;
    for (auto &s : f.samples) {
      float segH = (s.durationMs / maxMs) * graphH;
      float y0 = std::max(yBase - segH, gPos.y);
      draw->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, yBase), GetU32(s.name));
      yBase = y0;
    }
    if (sel)
      draw->AddRect(ImVec2(x0, gPos.y), ImVec2(x1, gPos.y + graphH),
                    IM_COL32(255, 220, 60, 230));
  }

  ImGui::InvisibleButton("##graph", ImVec2(gW, graphH));
  if (ImGui::IsItemHovered()) {
    float mx = ImGui::GetIO().MousePos.x - gPos.x;
    int slot = (int)(mx / barW);
    if (slot >= 0 && slot < PROFILER_HISTORY) {
      int hi = (curIdx + slot) % PROFILER_HISTORY;
      const auto &hf = history[hi];
      if (hf.totalMs > 0.f) {
        ImGui::BeginTooltip();
        ImGui::Text("Frame total: %.2f ms  (%.0f FPS)", hf.totalMs, hf.fps);
        ImGui::Separator();
        for (auto &s : hf.samples) {
          ImGui::PushStyleColor(ImGuiCol_Text, GetColor(s.name));
          ImGui::Text("  CPU %-20s %.3f ms", s.name.c_str(), s.durationMs);
          ImGui::PopStyleColor();
        }
        ImGui::EndTooltip();
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
          s_SelFrame = hi;
      }
    }
  }

  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();

  const ProfileFrame &frame =
      (s_SelFrame >= 0) ? history[s_SelFrame] : prof.GetLastFrame();
  float frameMs = frame.totalMs > 0.f ? frame.totalMs : 16.7f;

  const auto &gpuSamples = gpu.GetLastSamples();
  auto gpuMs = [&](const std::string &name) -> float {
    for (auto &g : gpuSamples)
      if (g.name == name)
        return g.durationMs;
    return -1.f;
  };

  const auto &sorted =
      frame.samples; 

  ImGui::TextDisabled("Frame: %.2f ms  |  %.1f FPS   |   GPU data: 1-frame "
                      "delayed (OpenGL async)",
                      frameMs, frame.fps);
  ImGui::Spacing();

  ImGui::PushStyleColor(ImGuiCol_TableRowBg, ImVec4(0.10f, 0.10f, 0.12f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt,
                        ImVec4(0.12f, 0.12f, 0.15f, 1.0f));

  if (ImGui::BeginTable("##ptable", 5,
                        ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg |
                            ImGuiTableFlags_SizingStretchProp,
                        ImVec2(0, tableH))) {

    ImGui::TableSetupColumn("Pass", ImGuiTableColumnFlags_WidthStretch, 0.22f);
    ImGui::TableSetupColumn("CPU ms", ImGuiTableColumnFlags_WidthFixed, 62.0f);
    ImGui::TableSetupColumn("CPU Bar", ImGuiTableColumnFlags_WidthStretch,
                            0.25f);
    ImGui::TableSetupColumn("GPU ms", ImGuiTableColumnFlags_WidthFixed, 62.0f);
    ImGui::TableSetupColumn("GPU Bar", ImGuiTableColumnFlags_WidthStretch,
                            0.30f);
    ImGui::TableHeadersRow();

    auto drawBar = [&](float ms, float maxForBar, ImU32 col, bool dimmed) {
      float ratio = std::min(ms / std::max(maxForBar, 0.001f), 1.f);
      ImVec2 rMin = ImGui::GetCursorScreenPos();
      float rW = ratio * (ImGui::GetContentRegionAvail().x - 4.f);
      ImU32 col2 =
          dimmed
              ? ImGui::ColorConvertFloat4ToU32(ImVec4(0.4f, 0.4f, 0.45f, 0.6f))
              : col;
      if (rW > 0)
        draw->AddRectFilled(rMin, ImVec2(rMin.x + rW, rMin.y + 15.f), col2,
                            2.f);
      if (rW > 28) {
        char pct[10];
        snprintf(pct, sizeof(pct), "%.1f%%", ratio * 100.f);
        draw->AddText(ImVec2(rMin.x + 4, rMin.y + 1),
                      IM_COL32(255, 255, 255, 180), pct);
      }
      ImGui::Dummy(ImVec2(0, 16));
    };

    float maxGpuMs = 1.0f;
    for (auto &g : gpuSamples)
      if (g.durationMs > maxGpuMs)
        maxGpuMs = g.durationMs;

    for (auto &s : sorted) {
      float gMs = gpuMs(s.name);
      ImVec4 col = GetColor(s.name);
      ImU32 u32 = ImGui::ColorConvertFloat4ToU32(col);

      ImGui::TableNextRow(0, 22.0f);

      ImGui::TableSetColumnIndex(0);
      ImGui::PushStyleColor(ImGuiCol_Text, col);
      ImGui::Text("  %s", s.name.c_str());
      ImGui::PopStyleColor();

      ImGui::TableSetColumnIndex(1);
      ImGui::Text("%.3f", s.durationMs);

      ImGui::TableSetColumnIndex(2);
      drawBar(s.durationMs, frameMs, u32, false);

      ImGui::TableSetColumnIndex(3);
      if (gMs >= 0.f)
        ImGui::Text("%.3f", gMs);
      else
        ImGui::TextDisabled("--");

      ImGui::TableSetColumnIndex(4);
      if (gMs >= 0.f)
        drawBar(gMs, maxGpuMs, u32, false);
      else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.4f, 0.4f, 1));
        ImGui::TextUnformatted("  (no GPU query)");
        ImGui::PopStyleColor();
      }
    }

    for (auto &g : gpuSamples) {
      bool found = false;
      for (auto &s : sorted)
        if (s.name == g.name) {
          found = true;
          break;
        }
      if (found)
        continue;

      ImVec4 col = GetColor(g.name);
      ImU32 u32 = ImGui::ColorConvertFloat4ToU32(col);
      ImGui::TableNextRow(0, 22.0f);
      ImGui::TableSetColumnIndex(0);
      ImGui::PushStyleColor(ImGuiCol_Text, col);
      ImGui::Text("  %s  (GPU only)", g.name.c_str());
      ImGui::PopStyleColor();
      ImGui::TableSetColumnIndex(1);
      ImGui::TextDisabled("--");
      ImGui::TableSetColumnIndex(2);
      ImGui::Dummy(ImVec2(0, 16));
      ImGui::TableSetColumnIndex(3);
      ImGui::Text("%.3f", g.durationMs);
      ImGui::TableSetColumnIndex(4);
      drawBar(g.durationMs, maxGpuMs, u32, false);
    }

    ImGui::EndTable();
  }
  ImGui::PopStyleColor(2);

  ImGui::Spacing();
  ImGui::Separator();
  float cpuTotal = 0.f;
  for (auto &s : frame.samples)
    cpuTotal += s.durationMs;
  float gpuTotal = 0.f;
  for (auto &g : gpuSamples)
    gpuTotal += g.durationMs;
  ImGui::TextDisabled("CPU sampled: %.2f ms  |  GPU sampled: %.2f ms  |  Frame "
                      "total: %.2f ms  [GL_TIMESTAMP queries]",
                      cpuTotal, gpuTotal, frameMs);
  ImGui::EndChild();
  ImGui::End();
}
#endif
