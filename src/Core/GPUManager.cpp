#include "GPUManager.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <unistd.h>
#include <cstring>

namespace fs = std::filesystem;

void GPUManager::EnsureProperGPU(int argc, char** argv) {
#ifdef __linux__
    if (IsPrimeOffloadEnabled()) {
        return;
    }

    int preferred = GetPreferredGPU();
    auto gpus = GetAvailableGPUs();
    
    GPUVendor vendor = GPUVendor::NONE;
    if (preferred >= 0 && preferred < gpus.size()) {
        vendor = gpus[preferred].vendor;
    } else {
        vendor = GetDetectedVendor();
    }

    if (vendor != GPUVendor::NONE) {
        if (vendor == GPUVendor::NVIDIA) {
            printf("[GPUManager] NVIDIA hardware detected. Re-executing with PRIME offload enabled...\n");
            setenv("NV_PRIME_RENDER_OFFLOAD", "1", 1);
            setenv("__NV_PRIME_RENDER_OFFLOAD", "1", 1);
            setenv("GLX_VENDOR_LIBRARY_NAME", "nvidia", 1);
            setenv("__GLX_VENDOR_LIBRARY_NAME", "nvidia", 1);
            setenv("__VK_LAYER_NV_optimus", "NVIDIA_only", 1);
        } else if (vendor == GPUVendor::AMD) {
            printf("[GPUManager] AMD hardware detected. Re-executing with DRI_PRIME=1...\n");
            setenv("DRI_PRIME", "1", 1);
        } else if (vendor == GPUVendor::INTEL) {
            printf("[GPUManager] Intel discrete hardware detected. Re-executing with DRI_PRIME=1...\n");
            setenv("DRI_PRIME", "1", 1);
        }

        char buf[1024];
        ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
        if (len != -1) {
            buf[len] = '\0';
            execvp(buf, argv);
        } else {
            execvp(argv[0], argv);
        }
        
        perror("execvp failed");
    }
#endif
}

std::vector<GPUManager::GPUInfo> GPUManager::GetAvailableGPUs() {
    std::vector<GPUInfo> gpus;
    try {
        int index = 0;
        for (const auto& entry : fs::directory_iterator("/sys/class/drm")) {
            std::string path = entry.path().string();
            
            if (path.find("card") != std::string::npos && path.find("-") == std::string::npos) {
                std::string vendorPath = path + "/device/vendor";
                std::ifstream vendorFile(vendorPath);
                if (vendorFile.is_open()) {
                    std::string vendorId;
                    vendorFile >> vendorId;
                    
                    GPUVendor v = GPUVendor::NONE;
                    std::string gpuName = "Unknown GPU";
                    
                    if (vendorId == "0x10de") {
                        v = GPUVendor::NVIDIA;
                        gpuName = "NVIDIA GeForce";
                    } else if (vendorId == "0x1002") {
                        v = GPUVendor::AMD;
                        gpuName = "AMD Radeon";
                    } else if (vendorId == "0x8086") {
                        v = GPUVendor::INTEL;
                        gpuName = "Intel Graphics";
                    } else {
                        gpuName = "Generic GPU (" + vendorId + ")";
                    }
                    
                    gpus.push_back({index++, v, gpuName + " (" + path.substr(path.find_last_of('/') + 1) + ")", vendorId});
                }
            }
        }
    } catch (...) {}
    return gpus;
}

void GPUManager::SetPreferredGPU(int index) {
    std::ofstream f("gpu_settings.cfg");
    f << index;
}

int GPUManager::GetPreferredGPU() {
    std::ifstream f("gpu_settings.cfg");
    int index = -1;
    if (f >> index) return index;
    return -1;
}

std::string GPUManager::GetVendorName(GPUVendor vendor) {
    switch (vendor) {
        case GPUVendor::NVIDIA: return "NVIDIA";
        case GPUVendor::AMD: return "AMD";
        case GPUVendor::INTEL: return "Intel";
        default: return "Unknown";
    }
}

GPUManager::GPUVendor GPUManager::GetDetectedVendor() {
    auto gpus = GetAvailableGPUs();
    if (gpus.empty()) return GPUVendor::NONE;
    
    
    for (const auto& gpu : gpus) {
        if (gpu.vendor == GPUVendor::NVIDIA) return GPUVendor::NVIDIA;
    }
    
    if (gpus.size() > 1) {
        return gpus[0].vendor; 
    }
    
    return GPUVendor::NONE;
}

bool GPUManager::IsPrimeOffloadEnabled() {
    if (getenv("NV_PRIME_RENDER_OFFLOAD")) return true;
    if (getenv("DRI_PRIME")) return true;
    return false;
}
