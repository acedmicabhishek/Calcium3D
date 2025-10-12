#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <imgui.h>
#include <string>
#include <vector>

class FileSystem {
public:
    FileSystem();
    void render();

private:
    void renderDirectoryTree(const std::string& path);
    void renderContentView();
    void createFile(const std::string& path);
    void createDirectory(const std::string& path);
    void deletePath(const std::string& path);
    void copyPath(const std::string& path);
    void pastePath(const std::string& destination);

    std::string currentPath;
    std::string selectedPath;
    std::string copiedPath;
};

#endif