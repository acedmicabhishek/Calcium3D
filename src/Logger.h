#pragma once

#include <vector>
#include <string>
#include "imgui.h"

class Logger {
public:
    static void AddLog(const char* fmt, ...);
    static void Draw(const char* title, bool* p_open = NULL);
private:
    static std::vector<std::string> buffer;
};
