#ifndef SCRIPT_COMPILER_H
#define SCRIPT_COMPILER_H

#include "Logger.h"
#include <cstdlib>
#include <dlfcn.h>
#include <filesystem>
#include <string>
#include <vector>

class ScriptCompiler {
public:
  static bool CompileAndLoad(const std::string &cppPath,
                             const std::string &engineRootPath) {
    namespace fs = std::filesystem;

    if (!fs::exists(cppPath)) {
      Logger::AddLog("[ScriptCompiler] File not found: %s", cppPath.c_str());
      return false;
    }

    std::string stem = fs::path(cppPath).stem().string();
    static int soCounter = 0;
    std::string soPath = "/tmp/calcium3d_script_" + stem + "_" +
                         std::to_string(soCounter++) + ".so";

    fs::path engineRoot = fs::path(engineRootPath);

    std::string includes;
    includes += " -I" + (engineRoot / "src").string();
    includes += " -I" + (engineRoot / "src" / "Core").string();
    includes += " -I" + (engineRoot / "src" / "Scene").string();
    includes += " -I" + (engineRoot / "src" / "Renderer").string();
    includes += " -I" + (engineRoot / "src" / "Editor").string();
    includes += " -I" + (engineRoot / "src" / "Environment").string();
    includes += " -I" + (engineRoot / "include" / "tinyobj").string();
    includes += " -I" + (engineRoot / "include").string();
    includes += " -I" + (engineRoot / "imgui").string();

    std::string cmd = "g++ -shared -fPIC -std=c++17 -O2" + includes + " -o " +
                      soPath + " " + cppPath +
                      " $(pkg-config --cflags glfw3) 2>&1";

    Logger::AddLog("[ScriptCompiler] Compiling: %s", stem.c_str());
    Logger::AddLog("[ScriptCompiler] CMD: %s", cmd.c_str());

    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
      Logger::AddLog("[ScriptCompiler] Failed to run compiler");
      return false;
    }

    char buffer[256];
    std::string output;
    while (fgets(buffer, sizeof(buffer), pipe)) {
      output += buffer;
    }
    int exitCode = pclose(pipe);

    if (exitCode != 0) {
      Logger::AddLog("[ScriptCompiler] Compilation FAILED:");
      Logger::AddLog("%s", output.c_str());
      return false;
    }

    Logger::AddLog("[ScriptCompiler] Compiled OK → %s", soPath.c_str());

    void *handle = dlopen(soPath.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if (!handle) {
      Logger::AddLog("[ScriptCompiler] dlopen FAILED: %s", dlerror());
      return false;
    }

    GetLoadedLibs().push_back(handle);
    Logger::AddLog("[ScriptCompiler] Loaded '%s' — behavior registered!",
                   stem.c_str());
    return true;
  }

  static bool IsCompiled(const std::string &scriptName) { return false; }

private:
  static std::vector<void *> &GetLoadedLibs() {
    static std::vector<void *> libs;
    return libs;
  }
};

#endif
