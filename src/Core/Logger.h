#pragma once

#include <vector>
#include <string>

#ifndef C3D_RUNTIME
#include "imgui.h"
#endif

class Console;

class Logger {
public:
    static void AddLog(const char* fmt, ...);
    static void Draw(const char* title, bool* p_open = NULL);
    static void SetRuntimeConsole(Console* console);
private:
    static std::vector<std::string> buffer;
    static Console* s_RuntimeConsole;
};
