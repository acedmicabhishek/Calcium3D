#include "filesystem.h"
#include <filesystem>
#include <iostream>
#include <fstream>

namespace fs = std::filesystem;

FileSystem::FileSystem() {
    currentPath = "../";
    selectedPath = "../";
}

void FileSystem::render() {
    ImGui::Begin("Project");

    ImGui::Columns(2, "FileSystemColumns", true);

    // Left panel: Directory Tree
    ImGui::BeginChild("DirectoryTree");
    renderDirectoryTree(currentPath);
    ImGui::EndChild();

    ImGui::NextColumn();

    // Right panel: Content View
    ImGui::BeginChild("ContentView");
    renderContentView();
    ImGui::EndChild();

    ImGui::Columns(1);

    ImGui::End();
}

void FileSystem::renderDirectoryTree(const std::string& path) {
    fs::path current_fs_path(path);

    for (const auto& entry : fs::directory_iterator(current_fs_path)) {
        if (entry.is_directory()) {
            const auto& p = entry.path();
            std::string filename = p.filename().string();
            
            ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
            if (selectedPath == p.string()) {
                node_flags |= ImGuiTreeNodeFlags_Selected;
            }

            bool node_open = ImGui::TreeNodeEx(filename.c_str(), node_flags);
            if (ImGui::IsItemClicked()) {
                selectedPath = p.string();
                currentPath = p.string();
            }

            if (node_open) {
                renderDirectoryTree(p.string());
                ImGui::TreePop();
            }
        }
    }
}

void FileSystem::renderContentView() {
    if (ImGui::Button("Back")) {
        fs::path p = currentPath;
        if (p.has_parent_path()) {
            currentPath = p.parent_path().string();
        }
    }
    ImGui::SameLine();
    ImGui::Text("Current Path: %s", currentPath.c_str());
    ImGui::Separator();

    float cellSize = 64.0f;
    float padding = 16.0f;
    float availableWidth = ImGui::GetContentRegionAvail().x;
    int columnCount = static_cast<int>(availableWidth / (cellSize + padding));
    if (columnCount < 1) columnCount = 1;

    ImGui::Columns(columnCount, 0, false);

    for (const auto& entry : fs::directory_iterator(currentPath)) {
        const auto& path = entry.path();
        std::string filename = path.filename().string();

        ImGui::Button(filename.c_str(), ImVec2(cellSize, cellSize));
        if (ImGui::IsItemClicked()) {
            selectedPath = path.string();
        }
        if (entry.is_directory() && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
            currentPath = path.string();
        }
        ImGui::TextWrapped("%s", filename.c_str());
        ImGui::NextColumn();
    }

    ImGui::Columns(1);

    if (ImGui::BeginPopupContextWindow("ContentViewContextMenu")) {
        if (ImGui::MenuItem("New File")) {
            createFile(currentPath + "/NewFile.txt");
        }
        if (ImGui::MenuItem("New Directory")) {
            createDirectory(currentPath + "/NewDirectory");
        }
        if (ImGui::MenuItem("Paste")) {
            pastePath(currentPath);
        }
        ImGui::EndPopup();
    }
}

void FileSystem::createFile(const std::string& path) {
    std::ofstream file(path);
    file.close();
}

void FileSystem::createDirectory(const std::string& path) {
    fs::create_directory(path);
}

void FileSystem::deletePath(const std::string& path) {
    fs::remove_all(path);
}

void FileSystem::copyPath(const std::string& path) {
    copiedPath = path;
}

void FileSystem::pastePath(const std::string& destination) {
    if (!copiedPath.empty()) {
        fs::path sourcePath = copiedPath;
        fs::path destPath = destination;
        destPath /= sourcePath.filename();
        fs::copy(sourcePath, destPath, fs::copy_options::recursive);
    }
}