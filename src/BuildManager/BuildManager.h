#ifndef BUILD_MANAGER_H
#define BUILD_MANAGER_H

#include <string>
#include <vector>
#include <filesystem>
#include <nlohmann/json.hpp>

class BuildManager {
public:
    struct BuildSettings {
        std::string OutputPath;
        std::string ProjectRoot;
        std::string TargetPlatform = "Linux";
        std::string StartScene = "main.scene";
        int StartGameState = 0; 
        std::map<int, std::string> CustomGameStates;
        nlohmann::json EnvironmentSettings;
        bool DisableStateWarning = false;
        bool EnableMasterControl = false;
        bool EnableHitboxes = false;
    };

    static bool Build(const BuildSettings& settings);

private:
    static bool CreateDirectoryStructure(const std::filesystem::path& root);
    static bool PrepareMesonSource(const std::filesystem::path& destination, const std::string& projectName);
    static bool CompileRuntime(const std::filesystem::path& buildDir, const std::string& projectName);
    static bool CopyProjectData(const std::filesystem::path& projectRoot, const std::filesystem::path& destination);
    static bool CopyEngineInternalData(const std::filesystem::path& destination);
    static bool GenerateConfigFile(const BuildSettings& settings);
};

#endif
