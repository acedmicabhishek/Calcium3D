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
    fs::create_directories(root / "Internal"); 
    return true;
}

bool BuildManager::PrepareMesonSource(const fs::path& destination, const std::string& projectName) {
    Logger::AddLog("Preparing Meson source code...");
    
    fs::path engineRoot = fs::current_path();
    while (!fs::exists(engineRoot / "Resource") && engineRoot.has_parent_path()) {
        engineRoot = engineRoot.parent_path();
    }
    
    
    if (fs::exists(engineRoot / "src")) {
        fs::copy(engineRoot / "src", destination / "src", fs::copy_options::recursive | fs::copy_options::overwrite_existing);
    }
    
    
    if (fs::exists(engineRoot / "imgui")) {
        fs::copy(engineRoot / "imgui", destination / "imgui", fs::copy_options::recursive | fs::copy_options::overwrite_existing);
    }

    
    if (fs::exists(engineRoot / "include")) {
        fs::copy(engineRoot / "include", destination / "include", fs::copy_options::recursive | fs::copy_options::overwrite_existing);
    }

    
    if (fs::exists(engineRoot / "fs")) {
        fs::copy(engineRoot / "fs", destination / "fs", fs::copy_options::recursive | fs::copy_options::overwrite_existing);
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
        out.close();

        
        fs::path scriptsDir = destination / "Scripts";
        if (fs::exists(scriptsDir)) {
            std::vector<std::string> userScripts;
            for (auto& entry : fs::recursive_directory_iterator(scriptsDir)) {
                if (entry.is_regular_file() && entry.path().extension() == ".cpp") {
                    std::string relPath = fs::relative(entry.path(), destination).string();
                    userScripts.push_back(relPath);
                    Logger::AddLog("  Found user script: %s", relPath.c_str());
                }
            }
            if (!userScripts.empty()) {
                
                std::ifstream mesonIn(destination / "meson.build");
                std::stringstream mesonBuf;
                mesonBuf << mesonIn.rdbuf();
                mesonIn.close();
                std::string mesonContent = mesonBuf.str();

                
                size_t sourcesEnd = mesonContent.rfind(")");
                
                size_t sourcesMarker = mesonContent.find("sources = files(");
                if (sourcesMarker != std::string::npos) {
                    
                    size_t closePos = mesonContent.find(")", sourcesMarker);
                    if (closePos != std::string::npos) {
                        std::string inject = ",\n";
                        for (size_t i = 0; i < userScripts.size(); i++) {
                            inject += "  '" + userScripts[i] + "'";
                            if (i < userScripts.size() - 1) inject += ",\n";
                            else inject += "\n";
                        }
                        mesonContent.insert(closePos, inject);
                        
                        std::ofstream mesonOut(destination / "meson.build");
                        mesonOut << mesonContent;
                        Logger::AddLog("  Injected %d user scripts into meson.build", (int)userScripts.size());
                    }
                }
            }
        }
    } else {
        Logger::AddLog("[ERROR] Meson template not found at %s", templatePath.c_str());
        return false;
    }

    return true;
}

bool BuildManager::CompileRuntime(const fs::path& buildDir, const std::string& projectName) {
    Logger::AddLog("Compiling standalone runtime with Meson...");
    
    
    if (!fs::exists(buildDir / "meson.build")) {
        Logger::AddLog("[ERROR] meson.build not found in %s! PrepareMesonSource may have failed.", buildDir.c_str());
        return false;
    }
    
    
    std::string setupCmd;
    if (fs::exists(buildDir / "build_runtime")) {
        setupCmd = "cd '" + buildDir.string() + "' && meson setup build_runtime --wipe";
    } else {
        setupCmd = "cd '" + buildDir.string() + "' && meson setup build_runtime";
    }
    std::string compileCmd = "cd '" + buildDir.string() + "' && meson compile -C build_runtime";
    
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
        Logger::AddLog("Binary copied to: %s", (buildDir / projectName).c_str());
        return true;
    }

    Logger::AddLog("[ERROR] Binary not found at: %s", binaryPath.c_str());
    return false;
}

bool BuildManager::CopyProjectData(const fs::path& projectRoot, const fs::path& destination) {
    Logger::AddLog("Copying project data from %s...", projectRoot.c_str());
    
    auto copyIfExists = [&](const std::string& folder) {
        fs::path src = projectRoot / folder;
        if (!fs::exists(src)) {
            
            std::string lower = folder;
            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
            src = projectRoot / lower;
        }

        if (fs::exists(src)) {
            try {
                fs::copy(src, destination / folder, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
                Logger::AddLog("  Bundled folder: %s", folder.c_str());
            } catch (const std::exception& e) {
                Logger::AddLog("  [WARNING] Failed to copy folder %s: %s", folder.c_str(), e.what());
            }
        } else {
            Logger::AddLog("  [INFO] Folder not found in project: %s", folder.c_str());
        }
    };

    copyIfExists("Assets");
    copyIfExists("Scenes");
    copyIfExists("Shaders");
    copyIfExists("Scripts");

    
    auto copyFileIfExists = [&](const std::string& filename) {
        fs::path src = projectRoot / filename;
        if (fs::exists(src)) {
            fs::copy_file(src, destination / filename, fs::copy_options::overwrite_existing);
            Logger::AddLog("  Copied %s to build folder.", filename.c_str());
        }
    };

    copyFileIfExists("ui_layout.json");
    copyFileIfExists("states.json");
    
    return true;
}

bool BuildManager::CopyEngineInternalData(const fs::path& destination) {
    Logger::AddLog("Copying engine internal resources...");
    
    fs::path engineRoot = fs::current_path();
    Logger::AddLog("  CWD: %s", fs::current_path().c_str());
    while (!fs::exists(engineRoot / "Resource") && engineRoot.has_parent_path()) {
        engineRoot = engineRoot.parent_path();
    }
    Logger::AddLog("  Resolved Engine Root: %s", engineRoot.c_str());
    
    fs::path resPath = engineRoot / "Resource";
    if (fs::exists(resPath)) {
        Logger::AddLog("  Copying Resource from: %s", resPath.c_str());
        fs::create_directories(destination / "Internal");
        fs::copy(resPath, destination / "Internal" / "Resource", fs::copy_options::recursive | fs::copy_options::overwrite_existing);
    } else {
        Logger::AddLog("  [WARNING] Resource directory not found at: %s", resPath.c_str());
    }

    fs::path shaderPath = engineRoot / "shaders";
    if (fs::exists(shaderPath)) {
        Logger::AddLog("  Copying shaders from: %s", shaderPath.c_str());
        fs::create_directories(destination / "Internal");
        fs::copy(shaderPath, destination / "Internal" / "shaders", fs::copy_options::recursive | fs::copy_options::overwrite_existing);
    } else {
        Logger::AddLog("  [WARNING] shaders directory not found at: %s", shaderPath.c_str());
    }

    return true;
}

bool BuildManager::GenerateConfigFile(const BuildSettings& settings) {
    Logger::AddLog("Generating project configuration...");
    
    fs::path configPath = fs::path(settings.OutputPath) / "project.json";
    
    nlohmann::json config;
    config["name"] = "Built Application";
    config["start_scene"] = settings.StartScene;
    config["start_state"] = settings.StartGameState;
    config["is_standalone"] = true;
    config["disable_state_warning"] = settings.DisableStateWarning;
    config["ms_control"] = settings.EnableMasterControl;
    config["hitboxes"] = settings.EnableHitboxes;
    
    nlohmann::json gameStatesObj = nlohmann::json::object();
    for (const auto& [id, name] : settings.CustomGameStates) {
        gameStatesObj[std::to_string(id)] = name;
    }
    config["game_states"] = gameStatesObj;
    
    if (!settings.EnvironmentSettings.empty()) {
        config["environment"] = settings.EnvironmentSettings;
    }
    if (!settings.GraphicsSettings.empty()) {
        config["graphics"] = settings.GraphicsSettings;
    }
    if (!settings.CameraSettings.empty()) {
        config["camera"] = settings.CameraSettings;
    }
    
    std::ofstream file(configPath);
    if (file.is_open()) {
        file << config.dump(4);
        file.close();
        return true;
    }
    
    return false;
}
