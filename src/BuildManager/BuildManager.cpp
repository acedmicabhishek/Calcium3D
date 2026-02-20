#include "BuildManager.h"
#include "../Core/Logger.h"
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

bool BuildManager::Build(const BuildSettings& settings) {
    fs::path outputRoot = settings.OutputPath;
    
    Logger::AddLog("--- Starting Build for %s ---", settings.TargetPlatform.c_str());
    Logger::AddLog("Output Path: %s", settings.OutputPath.c_str());

    try {
        if (!CreateDirectoryStructure(outputRoot)) return false;
        if (!CopyProjectData(settings.ProjectRoot, outputRoot)) return false;
        if (!CopyEngineInternalData(outputRoot)) return false;
        
        std::string projectName = fs::path(settings.ProjectRoot).filename().string();
        if (projectName.empty()) projectName = "CalciumGame";

        
        if (!PrepareMesonSource(outputRoot, projectName)) return false;
        if (!CompileRuntime(outputRoot, projectName)) return false;
        
        if (!GenerateConfigFile(settings)) return false;

        Logger::AddLog("--- Build Successful! ---");
        return true;
    } catch (const std::exception& e) {
        Logger::AddLog("[ERROR] Build failed: %s", e.what());
        return false;
    }
}

bool BuildManager::CreateDirectoryStructure(const fs::path& root) {
    Logger::AddLog("Creating directory structure...");
    if (fs::exists(root)) {
        fs::remove_all(root);
    }
    fs::create_directories(root);
    fs::create_directories(root / "Assets");
    fs::create_directories(root / "Scenes");
    fs::create_directories(root / "Shaders");
    fs::create_directories(root / "Internal");
    return true;
}

bool BuildManager::PrepareMesonSource(const fs::path& destination, const std::string& projectName) {
    Logger::AddLog("Preparing Meson source code...");
    
    fs::path engineRoot = fs::current_path().parent_path();
    
    
    if (fs::exists(engineRoot / "src")) {
        fs::copy(engineRoot / "src", destination / "src", fs::copy_options::recursive | fs::copy_options::overwrite_existing);
    }
    
    
    if (fs::exists(engineRoot / "imgui")) {
        fs::copy(engineRoot / "imgui", destination / "imgui", fs::copy_options::recursive | fs::copy_options::overwrite_existing);
    }

    
    if (fs::exists(engineRoot / "include")) {
        fs::copy(engineRoot / "include", destination / "include", fs::copy_options::recursive | fs::copy_options::overwrite_existing);
    }

    
    fs::path templatePath = engineRoot / "src" / "BuildManager" / "templates" / "linux" / "meson.build";
    if (fs::exists(templatePath)) {
        std::ifstream t(templatePath);
        std::stringstream buffer;
        buffer << t.rdbuf();
        std::string content = buffer.str();

        size_t pos = 0;
        while ((pos = content.find("calcium3d_player", pos)) != std::string::npos) {
            content.replace(pos, 16, projectName);
            pos += projectName.length();
        }

        std::ofstream out(destination / "meson.build");
        out << content;
    } else {
        Logger::AddLog("[ERROR] Meson template not found at %s", templatePath.c_str());
        return false;
    }

    return true;
}

bool BuildManager::CompileRuntime(const fs::path& buildDir, const std::string& projectName) {
    Logger::AddLog("Compiling standalone runtime with Meson...");
    
    std::string setupCmd = "cd " + buildDir.string() + " && meson setup build_runtime --wipe || meson setup build_runtime";
    std::string compileCmd = "cd " + buildDir.string() + " && meson compile -C build_runtime";
    
    Logger::AddLog("Running: %s", setupCmd.c_str());
    if (system(setupCmd.c_str()) != 0) {
        Logger::AddLog("[ERROR] Meson setup failed!");
        return false;
    }

    Logger::AddLog("Running: %s", compileCmd.c_str());
    if (system(compileCmd.c_str()) != 0) {
        Logger::AddLog("[ERROR] Meson compilation failed!");
        return false;
    }

    
    fs::path binaryPath = buildDir / "build_runtime" / projectName;
    if (fs::exists(binaryPath)) {
        fs::copy_file(binaryPath, buildDir / projectName, fs::copy_options::overwrite_existing);
        std::string chmodCmd = "chmod +x '" + (buildDir / projectName).string() + "'";
        system(chmodCmd.c_str());
        return true;
    }

    return false;
}

bool BuildManager::CopyProjectData(const fs::path& projectRoot, const fs::path& destination) {
    Logger::AddLog("Copying project data from %s...", projectRoot.c_str());
    
    auto copyIfExists = [&](const std::string& folder) {
        fs::path src = projectRoot / folder;
        if (fs::exists(src)) {
            fs::copy(src, destination / folder, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
        }
    };

    copyIfExists("Assets");
    copyIfExists("Scenes");
    copyIfExists("Shaders");
    
    return true;
}

bool BuildManager::CopyEngineInternalData(const fs::path& destination) {
    Logger::AddLog("Copying engine internal resources...");
    
    fs::path engineRoot = fs::current_path().parent_path();
    
    
    fs::path resPath = engineRoot / "Resource";
    if (fs::exists(resPath)) {
        fs::copy(resPath, destination / "Internal" / "Resource", fs::copy_options::recursive | fs::copy_options::overwrite_existing);
    }

    
    fs::path shaderPath = engineRoot / "shaders";
    if (fs::exists(shaderPath)) {
        fs::copy(shaderPath, destination / "Internal" / "shaders", fs::copy_options::recursive | fs::copy_options::overwrite_existing);
    }

    return true;
}

bool BuildManager::GenerateConfigFile(const BuildSettings& settings) {
    Logger::AddLog("Generating project configuration...");
    
    fs::path configPath = fs::path(settings.OutputPath) / "project.json";
    std::ofstream file(configPath);
    if (file.is_open()) {
        file << "{\n";
        file << "  \"name\": \"Built Application\",\n";
        file << "  \"start_scene\": \"" << settings.StartScene << "\",\n";
        file << "  \"is_standalone\": true\n";
        file << "}\n";
        file.close();
        return true;
    }
    
    return false;
}
