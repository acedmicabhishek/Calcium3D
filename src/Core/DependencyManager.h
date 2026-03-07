#pragma once
#include <vector>
#include <string>

struct DependencyInfo {
    std::string name;
    std::string packageName;     
    std::string pacmanPackage;   
    std::string aptPackage;      
    bool isInstalled;
    std::string description;
};

class DependencyManager {
public:
    static std::vector<DependencyInfo> GetDependencies();
    static bool InstallAll();
    static bool InstallPackage(const DependencyInfo& dep);
    static bool UninstallPackage(const DependencyInfo& dep);
    static bool IsInstalled(const std::string& packageName);
    static std::string GetPackageManager();
    
private:
    static bool RunWithSudo(const std::string& command);
};
