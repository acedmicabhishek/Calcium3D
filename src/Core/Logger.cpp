#include "Logger.h"
#include <cstdarg>
#include <iostream>
#include <string.h>

#ifndef C3D_RUNTIME
#include <imgui.h>
#endif

#include "Console.h"

std::vector<std::string> Logger::buffer;
Console *Logger::s_RuntimeConsole = nullptr;

void Logger::SetRuntimeConsole(Console *console) { s_RuntimeConsole = console; }

void Logger::AddLog(const char *fmt, ...) {
  char buf[1024];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf) / sizeof(buf[0]), fmt, args);
  buf[(sizeof(buf) / sizeof(buf[0])) - 1] = 0;
  va_end(args);
  buffer.push_back(buf);

#ifdef C3D_RUNTIME
  std::cout << "[LOG] " << buf << std::endl;
#endif

  if (buffer.size() > 100) {
    buffer.erase(buffer.begin());
  }

  if (s_RuntimeConsole) {
    s_RuntimeConsole->AddEngineLog(buf);
  }
}

void Logger::Draw(const char *title, bool *p_open) {
#ifndef C3D_RUNTIME
  ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin(title, p_open)) {
    ImGui::End();
    return;
  }
  if (ImGui::Button("Clear")) {
    buffer.clear();
  }
  ImGui::SameLine();
  bool copy = ImGui::Button("Copy");
  ImGui::Separator();
  ImGui::BeginChild("scrolling", ImVec2(0, 0), false,
                    ImGuiWindowFlags_HorizontalScrollbar);
  if (copy) {
    std::string fullLog;
    for (const auto &item : buffer) {
      fullLog += item + "\n";
    }
    ImGui::SetClipboardText(fullLog.c_str());
  }

  for (const auto &item : buffer) {
    ImGui::TextUnformatted(item.c_str());
  }

  if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
    ImGui::SetScrollHereY(1.0f);
  }

  ImGui::EndChild();
  ImGui::End();
#endif
}
