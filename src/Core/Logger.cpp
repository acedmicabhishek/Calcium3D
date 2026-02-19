#include "Logger.h"
#include <cstdarg>

std::vector<std::string> Logger::buffer;

void Logger::AddLog(const char* fmt, ...) {
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
    buf[IM_ARRAYSIZE(buf)-1] = 0;
    va_end(args);
    buffer.push_back(buf);
    
    
    if (buffer.size() > 100) {
        buffer.erase(buffer.begin());
    }
}

void Logger::Draw(const char* title, bool* p_open) {
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
    ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
    if (copy) {
        ImGui::LogToClipboard();
    }

    for (const auto& item : buffer) {
        ImGui::TextUnformatted(item.c_str());
    }

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();
    ImGui::End();
}
